/**
 * @file b_pdf_metadata.c
 * @brief PDF metadata parser and pretty-printer.
 */
#define _GNU_SOURCE
#include "./b_pdf_metadata.h"
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include "../../utils/ui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* =================== Helpers =================== */

static const char *find_backwards(const char *block, size_t size, const char *keyword)
{
    size_t klen = strlen(keyword);
    if (size < klen) return NULL;

    for (size_t i = size - klen; ; i--) {
        if (memcmp(block + i, keyword, klen) == 0)
            return block + i;
        if (i == 0) break;
    }
    return NULL;
}

static const char *find_forward(const char *start, const char *end, const char *keyword)
{
    size_t klen = strlen(keyword);
    size_t haystack_len = end - start;
    if (haystack_len < klen) return NULL;
    return memmem(start, haystack_len, keyword, klen);
}

static bool extract_dict_value(const char *dict_start, const char *dict_end,
                               const char *key, char *buf, size_t bufsize)
{
    char search[128];
    snprintf(search, sizeof(search), "/%s", key);
    size_t slen = strlen(search);

    const char *p = find_forward(dict_start, dict_end, search);
    if (!p) return false;

    p += slen;

    while (p < dict_end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
        p++;

    size_t i = 0;

    if (*p == '(') {
        const char *open = p;
        p++;
        int depth = 1;
        while (p < dict_end && i < bufsize - 1 && depth > 0) {
            if (*p == '\\') {
                /* Escaped character — copy both backslash and next char */
                if (i < bufsize - 2 && p + 1 < dict_end) {
                    buf[i++] = *p++;
                    buf[i++] = *p++;
                } else {
                    break;
                }
                continue;
            }
            if (*p == '(') depth++;
            else if (*p == ')') { depth--; if (depth == 0) break; }
            buf[i++] = *p++;
        }
        (void)open;
    } else if (*p == '<' && p + 1 < dict_end && *(p + 1) != '<') {
        p++;
        while (p < dict_end && *p != '>' && i < bufsize - 1)
            buf[i++] = *p++;
    } else {
        while (p < dict_end && i < bufsize - 1) {
            if (*p == '/' || (*p == '>' && p + 1 < dict_end && *(p + 1) == '>'))
                break;
            if (*p == '\r' || *p == '\n') break;
            buf[i++] = *p++;
        }
    }

    buf[i] = '\0';

    while (i > 0 && (buf[i - 1] == ' ' || buf[i - 1] == '\t'))
        buf[--i] = '\0';

    return i > 0;
}

static unsigned int count_occurrences(const char *block, size_t size, const char *pattern)
{
    unsigned int count = 0;
    size_t plen = strlen(pattern);
    if (size < plen) return 0;

    const char *p = block;
    const char *end = block + size;
    while ((p = memmem(p, end - p, pattern, plen)) != NULL) {
        count++;
        p += plen;
    }
    return count;
}

/**
 * @brief Count /Type /Page (but not /Type /Pages) occurrences in a single pass.
 */
static unsigned int count_pages(const char *block, size_t size)
{
    unsigned int count = 0;
    const char *p = block;
    const char *end = block + size;
    const char *type_str = "/Type";
    size_t type_len = 5;

    while ((p = memmem(p, end - p, type_str, type_len)) != NULL) {
        const char *q = p + type_len;

        /* Skip optional whitespace between /Type and /Page */
        while (q < end && (*q == ' ' || *q == '\t'))
            q++;

        if (q + 5 <= end && memcmp(q, "/Page", 5) == 0) {
            const char *after = q + 5;
            /* Exclude /Pages (the catalog object) */
            if (after < end && isalpha((unsigned char)*after)) {
                p = after;
                continue;
            }
            count++;
        }
        p = q;
    }
    return count;
}

/* =================== Metadata printers =================== */

static void print_pdf_header(const char *block, size_t size)
{
    printf(COLOR_BLUE "================= PDF Header ====================\n" COLOR_RESET);

    if (size >= 8 && memcmp(block, "%PDF-", 5) == 0) {
        char version[16] = {0};
        for (int i = 5; i < 15 && i < (int)size; i++) {
            if (block[i] == '\r' || block[i] == '\n') break;
            version[i - 5] = block[i];
        }
        printf(COLOR_GREEN "PDF Version:       " COLOR_RESET "%s\n", version);
    }

    const char *lin = find_forward(block, block + (size > 1024 ? 1024 : size), "/Linearized");
    printf(COLOR_GREEN "Linearized:        " COLOR_RESET "%s\n", lin ? "Yes" : "No");
}

static void print_pdf_trailer(const char *trailer, const char *block, size_t size)
{
    if (!trailer) {
        printf(COLOR_BLUE "\n================= Trailer ====================\n" COLOR_RESET);
        printf(COLOR_YELLOW "    (No traditional trailer found — may use cross-reference streams)\n" COLOR_RESET);
        return;
    }

    const char *end = block + size;
    const char *dict_start = find_forward(trailer, end, "<<");
    const char *dict_end = find_forward(trailer, end, ">>");
    if (!dict_start || !dict_end || dict_end <= dict_start) return;

    printf(COLOR_BLUE "\n================= Trailer ====================\n" COLOR_RESET);

    char buf[256];

    if (extract_dict_value(dict_start, dict_end, "Size", buf, sizeof(buf)))
        printf(COLOR_GREEN "    Size:          " COLOR_RESET "%s objects\n", buf);

    if (extract_dict_value(dict_start, dict_end, "Root", buf, sizeof(buf)))
        printf(COLOR_GREEN "    Root:          " COLOR_RESET "%s\n", buf);

    if (extract_dict_value(dict_start, dict_end, "Info", buf, sizeof(buf)))
        printf(COLOR_GREEN "    Info:          " COLOR_RESET "%s\n", buf);

    if (extract_dict_value(dict_start, dict_end, "Encrypt", buf, sizeof(buf)))
        printf(COLOR_GREEN "    Encrypt:       " COLOR_RESET "%s\n", buf);

    if (extract_dict_value(dict_start, dict_end, "ID", buf, sizeof(buf)))
        printf(COLOR_GREEN "    ID:            " COLOR_RESET "%s\n", buf);
}

static void print_pdf_xref(const char *block, size_t size)
{
    const char *xref = find_backwards(block, size, "startxref");
    if (!xref) return;

    printf(COLOR_BLUE "\n================= Cross-Reference ====================\n" COLOR_RESET);

    const char *p = xref + 9;
    const char *end = block + size;
    while (p < end && (*p == ' ' || *p == '\r' || *p == '\n'))
        p++;

    char offset_str[32] = {0};
    int i = 0;
    while (p < end && isdigit((unsigned char)*p) && i < 30)
        offset_str[i++] = *p++;

    if (i > 0)
        printf(COLOR_GREEN "    XRef Offset:   " COLOR_RESET "%s\n", offset_str);

    unsigned int xref_count = count_occurrences(block, size, "xref");
    unsigned int startxref_count = count_occurrences(block, size, "startxref");
    if (xref_count >= startxref_count)
        xref_count -= startxref_count;
    printf(COLOR_GREEN "    XRef Tables:   " COLOR_RESET "%u\n", xref_count);
}

static void print_pdf_info_dict(const char *trailer, const char *block, size_t size)
{
    if (!trailer) return;

    const char *end = block + size;
    const char *dict_start = find_forward(trailer, end, "<<");
    const char *dict_end = find_forward(trailer, end, ">>");
    if (!dict_start || !dict_end) return;

    char info_ref[64] = {0};
    if (!extract_dict_value(dict_start, dict_end, "Info", info_ref, sizeof(info_ref)))
        return;

    int obj_num = atoi(info_ref);
    if (obj_num <= 0) return;

    char obj_marker[32];
    snprintf(obj_marker, sizeof(obj_marker), "%d 0 obj", obj_num);

    const char *obj_start = find_forward(block, end, obj_marker);
    if (!obj_start) return;

    const char *obj_dict_start = find_forward(obj_start, end, "<<");
    const char *obj_end = find_forward(obj_start, end, "endobj");
    if (!obj_dict_start || !obj_end) return;

    const char *obj_dict_end = find_forward(obj_dict_start + 2, obj_end, ">>");
    if (!obj_dict_end) return;

    printf(COLOR_BLUE "\n================= Document Info ====================\n" COLOR_RESET);

    char buf[512];
    const char *keys[] = {"Title", "Author", "Subject", "Keywords", "Creator",
                          "Producer", "CreationDate", "ModDate", NULL};

    for (int k = 0; keys[k] != NULL; k++) {
        if (extract_dict_value(obj_dict_start, obj_dict_end, keys[k], buf, sizeof(buf)))
            printf(COLOR_GREEN "    %-15s" COLOR_RESET "%s\n", keys[k], buf);
    }
}

static void print_pdf_statistics(const char *block, size_t size)
{
    printf(COLOR_BLUE "\n================= Statistics ====================\n" COLOR_RESET);

    unsigned int obj_count = count_occurrences(block, size, " obj");
    printf(COLOR_GREEN "    Objects:       " COLOR_RESET "%u\n", obj_count);

    unsigned int stream_count = count_occurrences(block, size, "stream\r");
    stream_count += count_occurrences(block, size, "stream\n");
    unsigned int endstream_count = count_occurrences(block, size, "endstream");
    if (stream_count >= endstream_count)
        stream_count -= endstream_count;
    printf(COLOR_GREEN "    Streams:       " COLOR_RESET "%u\n", stream_count);

    unsigned int page_count = count_pages(block, size);
    printf(COLOR_GREEN "    Pages:         " COLOR_RESET "%u\n", page_count);

    bool encrypted = find_forward(block, block + size, "/Encrypt") != NULL;
    printf(COLOR_GREEN "    Encrypted:     " COLOR_RESET "%s\n", encrypted ? "Yes" : "No");

    printf(COLOR_GREEN "    File Size:     " COLOR_RESET "%zu bytes\n", size);
}

/* =================== Public entry point =================== */

bool b_pdf_metadata(bparser *parser, void *arg)
{
    if (!parser || !parser->block) {
        fprintf(stderr, COLOR_RED "[!] Invalid parser state\n" COLOR_RESET);
        return false;
    }

    const char *block = (const char *)parser->block;
    size_t size = parser->size;

    if (size < 8) {
        fprintf(stderr, COLOR_RED "[!] File too small to be a valid PDF\n" COLOR_RESET);
        return false;
    }

    if (memcmp(block, "%PDF-", 5) != 0) {
        fprintf(stderr, COLOR_RED "[!] Not a valid PDF file (missing %%PDF- header)\n" COLOR_RESET);
        return false;
    }

    const char *trailer = find_backwards(block, size, "trailer");

    print_pdf_header(block, size);
    print_pdf_trailer(trailer, block, size);
    print_pdf_xref(block, size);
    print_pdf_info_dict(trailer, block, size);
    print_pdf_statistics(block, size);

    printf("\n");
    return true;
}
