#include "./b_macho_metadata.h"
#include "../b_hashmap/b_hashmap.h"
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include "../bx_macho_utils/bx_macho_utils.h"

bool
b_macho_metadata(bparser* parser, void* arg)
{

    unsigned char *block = (unsigned char*) parser->block;
    mach_header *m_header = (mach_header*) block;


    if (m_header->magic == MH_MAGIC) {
        mach_header *m_header = (mach_header*) block;
        printf("32 bit mach-o binary file\n");
        printf("Cpu Type: %d", m_header->cputype);               /* cpu specifier */
        printf("cpusubtype: %d", m_header->cpusubtype);            /* machine specifier */
        printf("filetype: %d", m_header->filetype);              /* type of file */
        printf("ncmds: %d", m_header->ncmds);                 /* number of load commands */
        printf("sizeofcmds: %d", m_header->sizeofcmds);            /* the size of all the load commands */
        printf("flags: %d", m_header->flags);                 /* flags */

    } else if (m_header->magic == MH_MAGIC_64) {
        mach_header_64 *m_header = (mach_header_64*) block;

        printf("64 bit mach-o binary file\n");

        // m_header->cputype;               /* cpu specifier */
        // m_header->cpusubtype;            /* machine specifier */
        // m_header->filetype;              /* type of file */
        // m_header->ncmds;                 /* number of load commands */
        // m_header->sizeofcmds;            /* the size of all the load commands */
        // m_header->flags;                 /* flags */
        // m_header->reserved;              /* reserved */
        

        printf("Cpu Type: %d\n", m_header->cputype);               /* cpu specifier */
        printf("cpusubtype: %d\n", m_header->cpusubtype);            /* machine specifier */
        printf("filetype: %d\n", m_header->filetype);              /* type of file */
        printf("ncmds: %d\n", m_header->ncmds);                 /* number of load commands */
        printf("sizeofcmds: %d\n", m_header->sizeofcmds);            /* the size of all the load commands */
        printf("flags: %d\n", m_header->flags);                 /* flags */
        printf("reserved: %d\n", m_header->reserved);              /* reserved */

    } else {
        return false;
    }

    return true;
}
