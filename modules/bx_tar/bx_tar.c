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
    for (size_t i = 0; i < size_ptr && size_ptr[i]; i++) {
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
        unsigned int pos = 0;
        while (pos < parser->source.mem.size) {
            void* block = malloc(sizeof(posix_header));
            size_t n = bparser_read(parser, block, pos, sizeof(posix_header));
            if(n == 0) break;
            posix_header* header = (posix_header*)block;
            if (header -> name[0] == '\0') break;
            int file_size = oct2int(header->size);
            printf("=== TAR Header Info ===\n");
            printf("[  0] name      : %s\n", header->name);
            printf("[100] mode      : %s\n", header->mode);
            printf("[108] uid       : %s\n", header->uid);
            printf("[116] gid       : %s\n", header->gid);
            printf("[124] size      : %s\n", header->size);
            printf("[124] size      : %d\n", file_size);
            printf("[136] mtime     : %s\n", header->mtime);
            printf("[148] chksum    : %s\n", header->chksum);
            printf("[156] typeflag  : %c\n", header->typeflag);
            printf("[157] linkname  : %s\n", header->linkname);
            printf("[257] magic     : %s\n", header->magic);
            printf("[263] version   : %s\n", header->version);
            printf("[265] uname     : %s\n", header->uname);
            printf("[297] gname     : %s\n", header->gname);
            printf("[329] devmajor  : %s\n", header->devmajor);
            printf("[337] devminor  : %s\n", header->devminor);
            printf("[345] prefix    : %s\n", header->prefix);
            printf("========================\n\n");


            if(header ->typeflag == REGTYPE || header->typeflag == AREGTYPE) {
                printf("This is Reg File\n");
            }
            else if(header ->typeflag == LNKTYPE) {
                printf("This is LINK File\n");
            }
            else if(header ->typeflag == SYMTYPE ) {
                printf("This is reserved File\n");
            }
            else if(header ->typeflag == CHRTYPE) {
                printf("This is character special File\n");
            }
            else if(header ->typeflag == BLKTYPE) {
                printf("This is block special File\n");
            }
            else if(header ->typeflag == DIRTYPE) {
                printf("This is directory\n");
                // mkdir(header->name, oct2int(header->mode));
                pos+=BLOCK_SIZE;
                free(block);
                continue;
            }
            else if(header ->typeflag == FIFOTYPE) {
                printf("This is FIFO File\n");
            }
            else if(header ->typeflag == CONTTYPE) {
                printf("This is reserved File\n");
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
                printf("=== Block Content ===\n");
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
                for (char* byte = (char*)block_content; byte<(char*)block_content+block_size;byte++) {
                    printf("%02x", *byte);
                }

                printf("\n========================\n");
                free(block_content);
                pos += to_read;
            }
            // fclose(fp);
            // printf("Length Bytes: %d\n", file_size);

            free(block);
        }

        printf("Done\n");

    }
    return true;
}
