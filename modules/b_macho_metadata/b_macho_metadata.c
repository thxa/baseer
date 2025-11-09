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
        printf("Cpu Type: %d", m_header->cputype);             /* cpu specifier */
        printf("cpusubtype: %d", m_header->cpusubtype);        /* machine specifier */
        printf("filetype: %d", m_header->filetype);            /* type of file */
        printf("ncmds: %d", m_header->ncmds);                  /* number of load commands */
        printf("sizeofcmds: %d", m_header->sizeofcmds);        /* the size of all the load commands */
        printf("flags: %d", m_header->flags);                  /* flags */

    } else if (m_header->magic == MH_MAGIC_64) {
        mach_header_64 *m_header = (mach_header_64*) block;

        printf("================= Mach-O Header ====================\n");
        // printf("64 bit mach-o binary file\n");

        // m_header->cputype;               /* cpu specifier */
        // m_header->cpusubtype;            /* machine specifier */
        // m_header->filetype;              /* type of file */
        // m_header->ncmds;                 /* number of load commands */
        // m_header->sizeofcmds;            /* the size of all the load commands */
        // m_header->flags;                 /* flags */
        // m_header->reserved;              /* reserved */
        

        printf(COLOR_GREEN "Class: " COLOR_RESET "64-bit\n");

        printf(COLOR_GREEN "Cpu Type: " COLOR_RESET  "%d\n", m_header->cputype);                                     /* cpu specifier */
        printf(COLOR_GREEN "Machine: " COLOR_RESET "%d\n", m_header->cpusubtype);                              /* machine specifier */
        printf(COLOR_GREEN "File Type: " COLOR_RESET "%s\n",  get_mach_o_type(m_header->filetype));                                   /* type of file */
        printf(COLOR_GREEN "Number of load commands: " COLOR_RESET "%d\n", m_header->ncmds);                        /* number of load commands */
        printf(COLOR_GREEN "Size of all the load commands: " COLOR_RESET "%d\n", m_header->sizeofcmds);             /* the size of all the load commands */
        printf(COLOR_GREEN"Flags: "COLOR_RESET"\n", m_header->flags);  /* flags */ 
        print_mach_o_flags(m_header->flags);
        printf(COLOR_GREEN"Reserved: "COLOR_RESET"%d\n", m_header->reserved);                                    /* reserved */ 

        printf("================= Load Commands ====================\n");
        unsigned char *base_of_load_commands =  block + sizeof(mach_header_64);
        // printf("%p\n", &block);
        // printf("%p\n", &m_header);
        // printf("%p\n", &base_of_load_commands);


        unsigned char *offset_of_load_command =  base_of_load_commands;
        for (uint cmd_i =0; cmd_i< m_header->sizeofcmds;) {
            load_command *command = (load_command*)offset_of_load_command;
            // printf("Command: %d\n",command ->cmd);
            print_load_command_info(command->cmd);
            printf(COLOR_GREEN "Cmd Size:" COLOR_RESET " %d\n", command->cmdsize); 

            if(command->cmd == LC_SEGMENT_64) {
                segment_command_64  *seg_cmd = (segment_command_64*)offset_of_load_command;
                // seg_cmd->segname;	/* segment name */
                // seg_cmd->vmaddr;		/* memory address of this segment */
                // seg_cmd->vmsize;		/* memory size of this segment */
                // seg_cmd->fileoff;	/* file offset of this segment */
                // seg_cmd->filesize;	/* amount to map from the file */
                // seg_cmd->maxprot;	/* maximum VM protection */
                // seg_cmd->initprot;	/* initial VM protection */
                // seg_cmd->nsects;		/* number of sections in segment */
                // seg_cmd->flags;		/* flags */
                
                printf(COLOR_GREEN "Seg name:"COLOR_RESET" %s \n", seg_cmd->segname);	/* segment name */
                printf(COLOR_GREEN"VM address:"COLOR_RESET" 0x%llx\n", seg_cmd->vmaddr);		/* memory address of this segment */
                printf(COLOR_GREEN"VM size:"COLOR_RESET" %lld\n", seg_cmd->vmsize);		/* memory size of this segment */
                printf(COLOR_GREEN"File offset:"COLOR_RESET" 0x%llx\n", seg_cmd->fileoff);	/* file offset of this segment */
                printf(COLOR_GREEN"File size:"COLOR_RESET" %lld\n", seg_cmd->filesize);	/* amount to map from the file */
                printf(COLOR_GREEN"Max VM Protection:"COLOR_RESET" %x\n", seg_cmd->maxprot);	/* maximum VM protection */
                printf(COLOR_GREEN"Initial VM Protection:"COLOR_RESET" %x\n", seg_cmd->initprot);	/* initial VM protection */
                printf(COLOR_GREEN"Number of sections:"COLOR_RESET" %d\n", seg_cmd->nsects);		/* number of sections in segment */
                printf(COLOR_GREEN"Segment Flags"COLOR_RESET" (0x%08x):\n", seg_cmd->flags);		/* flags */     
                print_segment_flags(seg_cmd->flags);

                // for(char* sec_offset = block+seg_cmd->fileoff; sec_offset < block + seg_cmd->fileoff + seg_cmd->filesize;) {
                //     section_64* sec = (section_64*)

                //     // sec_offset += ;
                // }


            }
            printf("\n");

            cmd_i += command->cmdsize;
            offset_of_load_command += command->cmdsize;
        }

        printf("================= DATA ====================\n");


    } else {
        return false;
    }

    return true;
}
