/**
 * @file bx_pdf.c
 * @brief PDF format handler for Baseer.
 *
 * Dispatches PDF analysis based on command-line flags:
 *   -m  Metadata (header, trailer, xref, objects summary)
 */
#include "./bx_pdf.h"
#include "../../baseer.h"
#include "../b_pdf_metadata/b_pdf_metadata.h"
#include <string.h>
#include <stdio.h>

bool bx_pdf(bparser *parser, void *arg)
{
    int argc = *((inputs *)arg)->argc;
    char **args = ((inputs *)arg)->args;

    for (int i = 2; i < argc; i++) {
        if (strcmp("-m", args[i]) == 0) {
            bparser_apply(parser, b_pdf_metadata, arg);
        } else if (strcmp("--args", args[i]) == 0) {
            break;
        } else {
            fprintf(stderr, "[!] Unsupported flag for PDF: %s\n", args[i]);
        }
    }

    return true;
}
