#include "bx_tar.h"
#include <stdio.h>
#include <sys/stat.h>


typedef struct {
    /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    /* 500 */
    char pad[12];
} posix_header;

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */
#define BLOCK_SIZE 512

static inline int oct2int(char *size_ptr)
{
    int file_size = 0;
    for (size_t i = 0; i < (size_t)size_ptr && size_ptr[i]; i++) {
        if (size_ptr[i] >= '0' && size_ptr[i] <= '7')
            file_size = file_size * 8 + (size_ptr[i] - '0');
    }
    return file_size;
}

bool bx_tar(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;

    if(strcmp("-m", args[2]) == 0) {
        unsigned long long int pos = 0;
        while (pos < parser->size) {
            void* block = malloc(sizeof(posix_header));
            size_t n = bparser_read(parser, block, pos, sizeof(posix_header));
            if(n == 0) break;
            posix_header* header = (posix_header*)block;
            if (header -> name[0] == '\0') break;
            int file_size = oct2int(header->size);
            printf("\n=== TAR Header Info ===\n");
            printf(COLOR_GREEN "|--[  0] name      :%s %s\n", COLOR_RESET, header->name);
            printf(COLOR_GREEN "|--[100] mode      :%s %s\n", COLOR_RESET, header->mode);
            printf(COLOR_GREEN "|--[108] uid       :%s %s\n", COLOR_RESET, header->uid);
            printf(COLOR_GREEN "|--[116] gid       :%s %s\n", COLOR_RESET, header->gid);
            printf(COLOR_GREEN "|--[124] size      :%s %s\n", COLOR_RESET, header->size);
            printf(COLOR_GREEN "|--[124] size      :%s %d\n", COLOR_RESET, file_size);
            printf(COLOR_GREEN "|--[136] mtime     :%s %s\n", COLOR_RESET, header->mtime);
            printf(COLOR_GREEN "|--[148] chksum    :%s %s\n", COLOR_RESET, header->chksum);
            printf(COLOR_GREEN "|--[156] typeflag  :%s %c\n", COLOR_RESET, header->typeflag);
            printf(COLOR_GREEN "|--[157] linkname  :%s %s\n", COLOR_RESET, header->linkname);
            printf(COLOR_GREEN "|--[257] magic     :%s %s\n", COLOR_RESET, header->magic);
            printf(COLOR_GREEN "|--[263] version   :%s %s\n", COLOR_RESET, header->version);
            printf(COLOR_GREEN "|--[265] uname     :%s %s\n", COLOR_RESET, header->uname);
            printf(COLOR_GREEN "|--[297] gname     :%s %s\n", COLOR_RESET, header->gname);
            printf(COLOR_GREEN "|--[329] devmajor  :%s %s\n", COLOR_RESET, header->devmajor);
            printf(COLOR_GREEN "|--[337] devminor  :%s %s\n", COLOR_RESET, header->devminor);
            printf(COLOR_GREEN "|--[345] prefix    :%s %s\n", COLOR_RESET, header->prefix);

            if(header ->typeflag == REGTYPE || header->typeflag == AREGTYPE) {
                printf(COLOR_YELLOW "|---This is Reg File\n" COLOR_RESET);
            }
            else if(header ->typeflag == LNKTYPE) {
                printf(COLOR_YELLOW "|---This is LINK File\n" COLOR_RESET);
            }
            else if(header ->typeflag == SYMTYPE ) {
                printf(COLOR_YELLOW "|---This is reserved File\n" COLOR_RESET);
            }
            else if(header ->typeflag == CHRTYPE) {
                printf(COLOR_YELLOW "|---This is character special File\n" COLOR_RESET);
            }
            else if(header ->typeflag == BLKTYPE) {
                printf(COLOR_YELLOW "|---This is block special File\n" COLOR_RESET);
            }
            else if(header ->typeflag == DIRTYPE) {
                printf(COLOR_YELLOW "|---This is directory\n" COLOR_RESET);
                // mkdir(header->name, oct2int(header->mode));
                pos+=BLOCK_SIZE;
                free(block);
                continue;
            }
            else if(header ->typeflag == FIFOTYPE) {
                printf(COLOR_YELLOW "|---This is FIFO File\n" COLOR_RESET);
            }
            else if(header ->typeflag == CONTTYPE) {
                printf(COLOR_YELLOW "|---This is reserved File\n" COLOR_RESET);
            }

            unsigned int data_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
            unsigned int end_of_file = pos + BLOCK_SIZE + data_blocks * BLOCK_SIZE;

            pos += BLOCK_SIZE;

            if(file_size == 0) {
                pos = end_of_file;
                free(block);
                continue;
            }

            //             FILE* fp = fopen(header->name, "wb");
            //             if(!fp) {
            //                 perror("fopen");
            //                 pos = end_of_file;
            //                 free(block);
            //                 continue;
            //             }

            unsigned int remaining = file_size;
            while (pos < end_of_file) {
                unsigned int to_read = BLOCK_SIZE;
                if (end_of_file - pos < BLOCK_SIZE)
                    to_read = end_of_file - pos;

                void* block_content = malloc(to_read);
                size_t block_size = bparser_read(parser, block_content, pos, to_read);

                // if(block_size > 0) {
                //     size_t write_bytes = (remaining >= block_size) ? block_size : remaining;
                //     fwrite(block_content, 1, write_bytes, fp);
                //     remaining -= write_bytes;
                // }
                // for (char* byte = (char*)block_content; byte<(char*)block_content+block_size;byte++) {
                //     printf("%02x", *byte);
                // }

                char* ptr = (char*)block_content;
                printf(COLOR_GREEN "|" COLOR_RESET);
                print_hex_header((unsigned long long)pos);

                unsigned long long i, j;
                for (i = 0; ptr < (char*)block_content+block_size; i += BLOCK_LENGTH) {
                    // Print offset
                    printf(COLOR_GREEN "|----0x%08llx:  " COLOR_RESET,  pos + i);

                    // Print hex bytes
                    for (j = 0; j < BLOCK_LENGTH; j++) {
                        if (i + j < (size_t)(unsigned char*)block_content+block_size) {
                            const unsigned char c = ptr[i + j];
                            display_byte(&c);
                        // printf("%02x", ptr[i + j]);
                        } else {
                            printf("   "); // padding for alignment
                        }
                        if ((j + 1) % 2 == 0)
                            printf(" "); // extra space every 2 bytes
                    }

                    // Print ASCII chars
                    printf(" |");
                    for (j = 0; j < BLOCK_LENGTH && i + j < (size_t)(unsigned char*)block_content+block_size; j++) {
                        const unsigned char c = ptr[i + j];
                        display_byte_char(&c);
                    }
                    printf("|");
                    printf("\n");
                    ptr+=16;
                }

                free(block_content);
                pos += to_read;
            }
            // fclose(fp);
            // printf("Length Bytes: %d\n", file_size);

            free(block);
        }
    } else {
        fprintf(stderr, "[!] Unsupported flag: %s\n", args[2]);
        // printf(COLOR_YELLOW "[!] Not implemented "COLOR_RED"%s"COLOR_RESET COLOR_YELLOW" yet\n" COLOR_RESET, args[2]);
    }
    return true;
}
