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
        printf(COLOR_GREEN "Class: " COLOR_RESET "64-bit\n");
        printf(COLOR_GREEN "Cpu Type: " COLOR_RESET  "%d\n", m_header->cputype); /* cpu specifier */
        printf(COLOR_GREEN "Machine: " COLOR_RESET "%d\n", m_header->cpusubtype);/* machine specifier */
        printf(COLOR_GREEN "File Type: " COLOR_RESET "%s\n",  get_mach_o_type(m_header->filetype)); /* type of file */
        printf(COLOR_GREEN "Number of load commands: " COLOR_RESET "%d\n", m_header->ncmds); /* number of load commands */
        printf(COLOR_GREEN "Size of all the load commands: " COLOR_RESET "%d\n", m_header->sizeofcmds); /* the size of all the load commands */
        printf(COLOR_GREEN"Flags: "COLOR_RESET"\n", m_header->flags);  /* flags */ 
        print_mach_o_flags(m_header->flags);
        printf(COLOR_GREEN"Reserved: "COLOR_RESET"%d\n", m_header->reserved); /* reserved */ 
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
                print_macho_segment64_metadata(seg_cmd);
                unsigned char *seg_cmd_base = (unsigned char*) offset_of_load_command;
                if(seg_cmd_base + seg_cmd->cmdsize >=  parser->block+parser->size) {
                    puts("Out of bound attack detected\n");
                    break;
                }

                printf("\n");
                if(seg_cmd->nsects > 0) {
                    printf(COLOR_BLUE "Sections:\n" COLOR_RESET);
                    for(unsigned char* sec = seg_cmd_base + sizeof(segment_command_64); sec < seg_cmd_base + seg_cmd->cmdsize;
                                                           sec += sizeof(section_64)) { 
                        section_64* sec_cmd = (section_64*) sec;
                        print_macho_section64_metadata(sec_cmd);
                        printf("\n");
                    }
                    printf("\n");
                }

            } else if(command->cmd == LC_DYLD_CHAINED_FIXUPS) {
                /*
                 * The linkedit_data_command contains the offsets and sizes of a blob
                 * of data in the __LINKEDIT segment.  
                 */
                // print_linkedit_data_command_metadata()
                linkedit_data_command *led_cmd = (linkedit_data_command*)offset_of_load_command;
                printf(COLOR_GREEN "  Data offset:" COLOR_RESET " 0x%x\n", led_cmd->dataoff); 
                printf(COLOR_GREEN "  Data size:" COLOR_RESET " %d\n", led_cmd->datasize); 
                // struct linkedit_data_command {
                //     uint32_t	cmd;		/* LC_CODE_SIGNATURE, LC_SEGMENT_SPLIT_INFO,
                //                                    LC_FUNCTION_STARTS, LC_DATA_IN_CODE,
                //                                    LC_DYLIB_CODE_SIGN_DRS,
                //                                    LC_LINKER_OPTIMIZATION_HINT,
                //                                    LC_DYLD_EXPORTS_TRIE, or
                //                                    LC_DYLD_CHAINED_FIXUPS. */
                //     uint32_t	cmdsize;	/* sizeof(struct linkedit_data_command) */
                //     uint32_t	dataoff;	/* file offset of data in __LINKEDIT segment */
                //     uint32_t	datasize;	/* file size of data in __LINKEDIT segment  */
                // };

            } else if (command->cmd == LC_DYLD_EXPORTS_TRIE) {
                linkedit_data_command *led_cmd = (linkedit_data_command*)offset_of_load_command;
                printf(COLOR_GREEN "  Data offset:" COLOR_RESET " 0x%x\n", led_cmd->dataoff); 
                printf(COLOR_GREEN "  Data size:" COLOR_RESET " %d\n", led_cmd->datasize); 
            } else if(command->cmd == LC_SYMTAB) {

                /*
                 * The symtab_command contains the offsets and sizes of the link-edit 4.3BSD
                 * "stab" style symbol table information as described in the header files
                 * <nlist.h> and <stab.h>.
                 */
                // struct symtab_command {
                //     uint32_t	cmd;		/* LC_SYMTAB */
                //     uint32_t	cmdsize;	/* sizeof(struct symtab_command) */
                //     uint32_t	symoff;		/* symbol table offset */
                //     uint32_t	nsyms;		/* number of symbol table entries */
                //     uint32_t	stroff;		/* string table offset */
                //     uint32_t	strsize;	/* string table size in bytes */
                // };


            } else if(command->cmd == LC_DYSYMTAB) {

                /*
                 * This is the second set of the symbolic information which is used to support
                 * the data structures for the dynamically link editor.
                 *
                 * The original set of symbolic information in the symtab_command which contains
                 * the symbol and string tables must also be present when this load command is
                 * present.  When this load command is present the symbol table is organized
                 * into three groups of symbols:
                 *	local symbols (static and debugging symbols) - grouped by module
                 *	defined external symbols - grouped by module (sorted by name if not lib)
                 *	undefined external symbols (sorted by name if MH_BINDATLOAD is not set,
                 *	     			    and in order the were seen by the static
                 *				    linker if MH_BINDATLOAD is set)
                 * In this load command there are offsets and counts to each of the three groups
                 * of symbols.
                 *
                 * This load command contains a the offsets and sizes of the following new
                 * symbolic information tables:
                 *	table of contents
                 *	module table
                 *	reference symbol table
                 *	indirect symbol table
                 * The first three tables above (the table of contents, module table and
                 * reference symbol table) are only present if the file is a dynamically linked
                 * shared library.  For executable and object modules, which are files
                 * containing only one module, the information that would be in these three
                 * tables is determined as follows:
                 * 	table of contents - the defined external symbols are sorted by name
                 *	module table - the file contains only one module so everything in the
                 *		       file is part of the module.
                 *	reference symbol table - is the defined and undefined external symbols
                 *
                 * For dynamically linked shared library files this load command also contains
                 * offsets and sizes to the pool of relocation entries for all sections
                 * separated into two groups:
                 *	external relocation entries
                 *	local relocation entries
                 * For executable and object modules the relocation entries continue to hang
                 * off the section structures.
                 */
                // struct dysymtab_command {
                //     uint32_t cmd;	/* LC_DYSYMTAB */
                //     uint32_t cmdsize;	/* sizeof(struct dysymtab_command) */
                //     /*
                //      * The symbols indicated by symoff and nsyms of the LC_SYMTAB load command
                //      * are grouped into the following three groups:
                //      *    local symbols (further grouped by the module they are from)
                //      *    defined external symbols (further grouped by the module they are from)
                //      *    undefined symbols
                //      *
                //      * The local symbols are used only for debugging.  The dynamic binding
                //      * process may have to use them to indicate to the debugger the local
                //      * symbols for a module that is being bound.
                //      *
                //      * The last two groups are used by the dynamic binding process to do the
                //      * binding (indirectly through the module table and the reference symbol
                //      * table when this is a dynamically linked shared library file).
                //      */
                //     uint32_t ilocalsym;	/* index to local symbols */
                //     uint32_t nlocalsym;	/* number of local symbols */
                //     uint32_t iextdefsym;/* index to externally defined symbols */
                //     uint32_t nextdefsym;/* number of externally defined symbols */
                //     uint32_t iundefsym;	/* index to undefined symbols */
                //     uint32_t nundefsym;	/* number of undefined symbols */
                //     /*
                //      * For the for the dynamic binding process to find which module a symbol
                //      * is defined in the table of contents is used (analogous to the ranlib
                //      * structure in an archive) which maps defined external symbols to modules
                //      * they are defined in.  This exists only in a dynamically linked shared
                //      * library file.  For executable and object modules the defined external
                //      * symbols are sorted by name and is use as the table of contents.
                //      */
                //     uint32_t tocoff;	/* file offset to table of contents */
                //     uint32_t ntoc;	/* number of entries in table of contents */
                //     /*
                //      * To support dynamic binding of "modules" (whole object files) the symbol
                //      * table must reflect the modules that the file was created from.  This is
                //      * done by having a module table that has indexes and counts into the merged
                //      * tables for each module.  The module structure that these two entries
                //      * refer to is described below.  This exists only in a dynamically linked
                //      * shared library file.  For executable and object modules the file only
                //      * contains one module so everything in the file belongs to the module.
                //      */
                //     uint32_t modtaboff;	/* file offset to module table */
                //     uint32_t nmodtab;	/* number of module table entries */
                //     /*
                //      * To support dynamic module binding the module structure for each module
                //      * indicates the external references (defined and undefined) each module
                //      * makes.  For each module there is an offset and a count into the
                //      * reference symbol table for the symbols that the module references.
                //      * This exists only in a dynamically linked shared library file.  For
                //      * executable and object modules the defined external symbols and the
                //      * undefined external symbols indicates the external references.
                //      */
                //     uint32_t extrefsymoff;	/* offset to referenced symbol table */
                //     uint32_t nextrefsyms;	/* number of referenced symbol table entries */
                //     /*
                //      * The sections that contain "symbol pointers" and "routine stubs" have
                //      * indexes and (implied counts based on the size of the section and fixed
                //      * size of the entry) into the "indirect symbol" table for each pointer
                //      * and stub.  For every section of these two types the index into the
                //      * indirect symbol table is stored in the section header in the field
                //      * reserved1.  An indirect symbol table entry is simply a 32bit index into
                //      * the symbol table to the symbol that the pointer or stub is referring to.
                //      * The indirect symbol table is ordered to match the entries in the section.
                //      */
                //     uint32_t indirectsymoff; /* file offset to the indirect symbol table */
                //     uint32_t nindirectsyms;  /* number of indirect symbol table entries */
                //     /*
                //      * To support relocating an individual module in a library file quickly the
                //      * external relocation entries for each module in the library need to be
                //      * accessed efficiently.  Since the relocation entries can't be accessed
                //      * through the section headers for a library file they are separated into
                //      * groups of local and external entries further grouped by module.  In this
                //      * case the presents of this load command who's extreloff, nextrel,
                //      * locreloff and nlocrel fields are non-zero indicates that the relocation
                //      * entries of non-merged sections are not referenced through the section
                //      * structures (and the reloff and nreloc fields in the section headers are
                //      * set to zero).
                //      *
                //      * Since the relocation entries are not accessed through the section headers
                //      * this requires the r_address field to be something other than a section
                //      * offset to identify the item to be relocated.  In this case r_address is
                //      * set to the offset from the vmaddr of the first LC_SEGMENT command.
                //      * For MH_SPLIT_SEGS images r_address is set to the the offset from the
                //      * vmaddr of the first read-write LC_SEGMENT command.
                //      *
                //      * The relocation entries are grouped by module and the module table
                //      * entries have indexes and counts into them for the group of external
                //      * relocation entries for that the module.
                //      *
                //      * For sections that are merged across modules there must not be any
                //      * remaining external relocation entries for them (for merged sections
                //      * remaining relocation entries must be local).
                //      */
                //     uint32_t extreloff;	/* offset to external relocation entries */
                //     uint32_t nextrel;	/* number of external relocation entries */
                //     /*
                //      * All the local relocation entries are grouped together (they are not
                //      * grouped by their module since they are only used if the object is moved
                //      * from it staticly link edited address).
                //      */
                //     uint32_t locreloff;	/* offset to local relocation entries */
                //     uint32_t nlocrel;	/* number of local relocation entries */
                // };


            // } else if(command->cmd == LC_LOAD_DYLINKER) {
            } else if(command->cmd == LC_ID_DYLINKER || command->cmd ==  LC_LOAD_DYLINKER || command->cmd == LC_DYLD_ENVIRONMENT  ) {
                dylinker_command *dyld_cmd = (dylinker_command*)offset_of_load_command;
                printf("====================hello\n");
                // printf(COLOR_GREEN "  " COLOR_RESET " %s\n", dyld_cmd->name);
                /*
                 * A program that uses a dynamic linker contains a dylinker_command to identify
                 * the name of the dynamic linker (LC_LOAD_DYLINKER).  And a dynamic linker
                 * contains a dylinker_command to identify the dynamic linker (LC_ID_DYLINKER).
                 * A file can have at most one of these.
                 * This struct is also used for the LC_DYLD_ENVIRONMENT load command and
                 * contains string for dyld to treat like environment variable.
                 */
                // struct dylinker_command {
                //     uint32_t	cmd;		/* LC_ID_DYLINKER, LC_LOAD_DYLINKER or
                //                                    LC_DYLD_ENVIRONMENT */
                //     uint32_t	cmdsize;	/* includes pathname string */
                //     union lc_str    name;		/* dynamic linker's path name */
                // };


            } else if(command->cmd == LC_UUID) {

                /*
                 * The uuid load command contains a single 128-bit unique random number that
                 * identifies an object produced by the static link editor.
                 */
                // struct uuid_command {
                //     uint32_t	cmd;		/* LC_UUID */
                //     uint32_t	cmdsize;	/* sizeof(struct uuid_command) */
                //     uint8_t	uuid[16];	/* the 128-bit uuid */
                // };

                uuid_command *uuid_cmd = (uuid_command*)offset_of_load_command;
                printf(COLOR_GREEN"UUID: "COLOR_RESET);
                print_uuid(uuid_cmd->uuid);
                printf("\n");

            } else if(command->cmd == LC_BUILD_VERSION) {

                /*
                 * The build_version_command contains the min OS version on which this
                 * binary was built to run for its platform.  The list of known platforms and
                 * tool values following it.
                 */
                // struct build_version_command {
                //     uint32_t	cmd;		/* LC_BUILD_VERSION */
                //     uint32_t	cmdsize;	/* sizeof(struct build_version_command) plus */
                //     /* ntools * sizeof(struct build_tool_version) */
                //     uint32_t	platform;	/* platform */
                //     uint32_t	minos;		/* X.Y.Z is encoded in nibbles xxxx.yy.zz */
                //     uint32_t	sdk;		/* X.Y.Z is encoded in nibbles xxxx.yy.zz */
                //     uint32_t	ntools;		/* number of tool entries following this */
                // };
                //
                //typedef struct build_tool_version {
                // uint32_t	tool;		/* enum for the tool */
                // uint32_t	version;	/* version number of the tool */
                // } build_tool_version ;

                void *build_v_cmd_tools_ptr = (void*)offset_of_load_command + sizeof(build_version_command);
                build_version_command *build_v_cmd = (build_version_command*)offset_of_load_command;

                printf(COLOR_GREEN "Platform: " COLOR_RESET "%s\n", platform_to_string(build_v_cmd->platform));
                printf(COLOR_GREEN "Min OS: " COLOR_RESET); print_tool_version(build_v_cmd->minos); printf("\n");
                printf(COLOR_GREEN "SDK: " COLOR_RESET); print_tool_version(build_v_cmd->sdk); printf("\n");

                if (build_v_cmd -> ntools > 0) {
                    build_tool_version *tools = (build_tool_version *)build_v_cmd_tools_ptr;
                    for (int i = 0; i < build_v_cmd -> ntools; i++) {
                        printf(COLOR_GREEN "Tool: " COLOR_RESET "%s, "COLOR_GREEN"Version: " COLOR_RESET, tool_to_string(tools[i].tool));
                        print_tool_version(tools[i].version);
                        printf("\n");
                    }
                }

            } else if(command->cmd == LC_SOURCE_VERSION) {

                /*
                 * The source_version_command is an optional load command containing
                 * the version of the sources used to build the binary.
                 */
                // struct source_version_command {
                //     uint32_t  cmd;	/* LC_SOURCE_VERSION */
                //     uint32_t  cmdsize;	/* 16 */
                //     uint64_t  version;	/* A.B.C.D.E packed as a24.b10.c10.d10.e10 */
                // };
                source_version_command * svc = (source_version_command *) offset_of_load_command;
                
                //
                printf(COLOR_GREEN "Version: " COLOR_RESET);
                print_source_version(svc->version);
                printf("\n");
                // printf(COLOR_GREEN "version: " COLOR_RESET "%s\n", svc->version) ;


            } else if(command->cmd == LC_MAIN) {


                /*
                 * The entry_point_command is a replacement for thread_command.
                 * It is used for main executables to specify the location (file offset)
                 * of main().  If -stack_size was used at link time, the stacksize
                 * field will contain the stack size need for the main thread.
                 */
                // struct entry_point_command {
                //     uint32_t  cmd;	/* LC_MAIN only used in MH_EXECUTE filetypes */
                //     uint32_t  cmdsize;	/* 24 */
                //     uint64_t  entryoff;	/* file (__TEXT) offset of main() */
                //     uint64_t  stacksize;/* if not zero, initial stack size */
                // };
                
                entry_point_command* epc = (entry_point_command*) offset_of_load_command;
                printf(COLOR_GREEN "Entry offset: " COLOR_RESET "0x%08lx\n", epc->entryoff );
                printf(COLOR_GREEN "Stack size: " COLOR_RESET "0x%ld\n", epc->stacksize);

            } else if(command->cmd == LC_LOAD_DYLIB) {

                /*
                 * A dynamically linked shared library (filetype == MH_DYLIB in the mach header)
                 * contains a dylib_command (cmd == LC_ID_DYLIB) to identify the library.
                 * An object that uses a dynamically linked shared library also contains a
                 * dylib_command (cmd == LC_LOAD_DYLIB, LC_LOAD_WEAK_DYLIB, or
                 * LC_REEXPORT_DYLIB) for each library it uses.
                 */
                // struct dylib_command {
                //     uint32_t	cmd;		/* LC_ID_DYLIB, LC_LOAD_{,WEAK_}DYLIB,
                //                                    LC_REEXPORT_DYLIB */
                //     uint32_t	cmdsize;	/* includes pathname string */
                //     struct dylib	dylib;		/* the library identification */
                // };
                //

                dylib_command *dylib_cmd = (dylib_command *) offset_of_load_command;
                // struct dylib dylib = &dylib_cmd->dylib;
                // printf("dylib: %s\n", dylib->name);
                // printf("%d\n", dylib.current_version);
                // printf("%d\n", dylib.compatibility_version);
                // printf("%d\n", dylib.timestamp);




            } else if(command->cmd == LC_FUNCTION_STARTS) {

                /*
                 * The linkedit_data_command contains the offsets and sizes of a blob
                 * of data in the __LINKEDIT segment.  
                 */
                // struct linkedit_data_command {
                //     uint32_t	cmd;		/* LC_CODE_SIGNATURE, LC_SEGMENT_SPLIT_INFO,
                //                                    LC_FUNCTION_STARTS, LC_DATA_IN_CODE,
                //                                    LC_DYLIB_CODE_SIGN_DRS,
                //                                    LC_LINKER_OPTIMIZATION_HINT,
                //                                    LC_DYLD_EXPORTS_TRIE, or
                //                                    LC_DYLD_CHAINED_FIXUPS. */
                //     uint32_t	cmdsize;	/* sizeof(struct linkedit_data_command) */
                //     uint32_t	dataoff;	/* file offset of data in __LINKEDIT segment */
                //     uint32_t	datasize;	/* file size of data in __LINKEDIT segment  */
                // };


            } else if(command->cmd == LC_DATA_IN_CODE) {

                /*
                 * The LC_DATA_IN_CODE load commands uses a linkedit_data_command 
                 * to point to an array of data_in_code_entry entries. Each entry
                 * describes a range of data in a code section.
                 */
                // struct data_in_code_entry {
                //     uint32_t	offset;  /* from mach_header to start of data range*/
                //     uint16_t	length;  /* number of bytes in data range */
                //     uint16_t	kind;    /* a DICE_KIND_* value  */
                // };

            } else if(command->cmd == LC_CODE_SIGNATURE) {
                
                // /*
                //  * The linkedit_data_command contains the offsets and sizes of a blob
                //  * of data in the __LINKEDIT segment.  
                //  */
                // struct linkedit_data_command {
                //     uint32_t	cmd;		/* LC_CODE_SIGNATURE, LC_SEGMENT_SPLIT_INFO,
                //                                    LC_FUNCTION_STARTS, LC_DATA_IN_CODE,
                // 				   LC_DYLIB_CODE_SIGN_DRS,
                // 				   LC_LINKER_OPTIMIZATION_HINT,
                // 				   LC_DYLD_EXPORTS_TRIE, or
                // 				   LC_DYLD_CHAINED_FIXUPS. */
                //     uint32_t	cmdsize;	/* sizeof(struct linkedit_data_command) */
                //     uint32_t	dataoff;	/* file offset of data in __LINKEDIT segment */
                //     uint32_t	datasize;	/* file size of data in __LINKEDIT segment  */
                // };

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
