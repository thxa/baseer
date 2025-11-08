#ifndef BX_MACHO_UTILS
#define BX_MACHO_UTILS
#include <stdint.h>

// https://opendlang.org/library/source/core.sys.darwin.mach.loader.d.html#L439
typedef int cpu_type_t;
typedef int cpu_subtype_t;
typedef unsigned int uint;
typedef unsigned long ulong;

#define MH_MAGIC 0xfeedface /* the mach magic number */
#define MH_MAGIC_64 0xfeedfacf /* the 64-bit mach magic number */

// #define FAT_MAGIC 0xcafebabe
// #define FAT_CIGAM 0xbebafeca /* NXSwapLong(FAT_MAGIC) */


typedef struct fat_header {
    uint32_t magic; /* FAT_MAGIC */
    uint32_t nfat_arch; /* number of structs that follow */
} fat_header;

typedef struct fat_arch {
    cpu_type_t cputype; /* cpu specifier (int) */
    cpu_subtype_t cpusubtype; /* machine specifier (int) */
    uint32_t offset; /* file offset to this object file */
    uint32_t size; /* size of this object file */
    uint32_t align; /* alignment as a power of 2 */
} fat_arch;


typedef struct mach_header {
    uint32_t        magic;                 /* mach magic number identifier */
    cpu_type_t      cputype;               /* cpu specifier */
    cpu_subtype_t   cpusubtype;            /* machine specifier */
    uint32_t        filetype;              /* type of file */
    uint32_t        ncmds;                 /* number of load commands */
    uint32_t        sizeofcmds;            /* the size of all the load commands */
    uint32_t        flags;                 /* flags */
} mach_header;

typedef struct mach_header_64 {
    uint32_t        magic;                 /* mach magic number identifier */
    cpu_type_t      cputype;               /* cpu specifier */
    cpu_subtype_t   cpusubtype;            /* machine specifier */
    uint32_t        filetype;              /* type of file */
    uint32_t        ncmds;                 /* number of load commands */
    uint32_t        sizeofcmds;            /* the size of all the load commands */
    uint32_t        flags;                 /* flags */
    uint32_t        reserved;              /* reserved */
} mach_header_64;

/**
 * The layout of the file depends on the filetype. For all but the MH_OBJECT
 * file type the segments are padded out and aligned on a segment alignment
 * boundary for efficient demand pageing. The MH_EXECUTE, MH_FVMLIB,
 * MH_DYLIB, MH_DYLINKER and MH_BUNDLE file types also have the headers
 * included as part of their first segment.
 *
 * The file type MH_OBJECT is a compact format intended as output of the
 * assembler and input (and possibly output) of the link editor (the .o
 * format). All sections are in one unnamed segment with no segment padding.
 * This format is used as an executable format when the file is so small the
 * segment padding greatly increases its size.
 *
 * The file type MH_PRELOAD is an executable format intended for things that
 * are not executed under the kernel (proms, stand alones, kernels, etc).
 * The format can be executed under the kernel but may demand paged it and
 * not preload it before execution.
 *
 * A core file is in MH_CORE format and can be any in an arbitrary legal
 * Mach-O file.
 *
 * Constants for the filetype field of the mach_header
 */
enum
{
    /// Relocatable object file.
    MH_OBJECT,

    /// Demand paged executable file.
    MH_EXECUTE,

    /// Fixed VM shared library file.
    MH_FVMLIB,

    /// Core file.
    MH_CORE,

    /// Preloaded executable file.
    MH_PRELOAD,

    /// Dynamically bound shared library.
    MH_DYLIB,

    /// Dynamic link editor.
    MH_DYLINKER,

    /// Dynamically bound bundle file.
    MH_BUNDLE,

    /// Shared library stub for static linking only, no section contents.
    MH_DYLIB_STUB,

    /// Companion file with only debug sections.
    MH_DSYM,

    /// X86_64 kexts.
    MH_KEXT_BUNDLE
};

typedef struct load_command {
    uint32_t cmd;       /* type of load command */
    uint32_t cmdsize;   /* total size of command in bytes */
} load_command;


/**
 * The segment load command indicates that a part of this file is to be
 * mapped into the task's address space. The size of this segment in memory,
 * vmsize, maybe equal to or larger than the amount to map from this file,
 * filesize. The file is mapped starting at fileoff to the beginning of
 * the segment in memory, vmaddr. The rest of the memory of the segment,
 * if any, is allocated zero fill on demand. The segment's maximum virtual
 * memory protection and initial virtual memory protection are specified
 * by the maxprot and initprot fields. If the segment has sections then the
 * section structures directly follow the segment command and their size is
 * reflected in cmdsize.
 */
typedef struct segment_command {
    uint cmd; /// LC_SEGMENT.
    uint cmdsize; /// Includes sizeof section structs.
    char segname[16]; /// Segment name.
    uint vmaddr; /// Memory address of this segment.
    uint vmsize; /// Memory size of this segment.
    uint fileoff; /// File offset of this segment.
    uint filesize; /// Amount to map from the file.
    int maxprot; /// Maximum VM protection.
    int initprot; /// Initial VM protection.
    uint nsects; /// Number of sections in segment.
    uint flags; /// Flags.
} segment_command;


/*
 * The 64-bit segment load command indicates that a part of this file is to
 * be mapped into a 64-bit task's address space. If the 64-bit segment has
 * sections then section_64 structures directly follow the 64-bit segment
 * command and their size is reflected in cmdsize.
 */
typedef struct segment_command_64 { 
    uint cmd; /// LC_SEGMENT_64.
    uint cmdsize; /// Includes sizeof section_64 structs.
    char segname[16]; /// Segment name.
    ulong vmaddr; /// Memory address of this segment.
    ulong vmsize; /// Memory size of this segment.
    ulong fileoff; /// File offset of this segment.
    ulong filesize; /// Amount to map from the file.
    int maxprot; /// Maximum VM protection.
    int initprot; /// Initial VM protection.
    uint nsects; /// Number of sections in segment.
    uint flags; /// Flags.
} segment_command_64;


// typedef struct segment_command_64 { /* for 64-bit architectures */
//     uint32_t    cmd;           /* LC_SEGMENT_64 */
//     uint32_t    cmdsize;       /* includes sizeof section_64 structs */
//     char        segname[16];   /* segment name */
//     uint64_t    vmaddr;        /* memory address of this segment */
//     uint64_t    vmsize;        /* memory size of this segment */
//     uint64_t    fileoff;       /* file offset of this segment */
//     uint64_t    filesize;      /* amount to map from the file */
//     vm_prot_t   maxprot;       /* maximum VM protection */
//     vm_prot_t   initprot;      /* initial VM protection */
//     uint32_t    nsects;        /* number of sections in segment */
//     uint32_t    flags;         /* flags */
// } segment_command_64 ;

/**
 * A segment is made up of zero or more sections. Non-MH_OBJECT files have
 * all of their segments with the proper sections in each, and padded to the
 * specified segment alignment when produced by the link editor. The first
 * segment of a MH_EXECUTE and MH_FVMLIB format file contains the
 * mach_header and load commands of the object file before its first
 * section. The zero fill sections are always last in their segment
 * (in all formats). This allows the zeroroed segment padding to be mapped
 * into memory where zero fill sections might be. The gigabyte zero fill
 * sections, those with the section type S_GB_ZEROFILL, can only be in a
 * segment with sections of this type. These segments are then placed after
 * all other segments.
 *
 * The MH_OBJECT format has all of its sections in one segment for
 * compactness. There is no padding to a specified segment boundary and the
 * mach_header and load commands are not part of the segment.
 *
 * Sections with the same section name, sectname, going into the same
 * segment, segname, are combined by the link editor. The resulting section,
 * is aligned to the maximum alignment of the combined sections and is the
 * new section's alignment. The combined sections are aligned to their
 * original alignment in the combined section. Any padded bytes to get the
 * specified alignment are zeroed.
 *
 * The format of the relocation entries referenced by the reloff and nreloc
 * fields of the section structure for mach object files is described in the
 * header file <reloc.h>.
 */
typedef struct section
{
    char sectname[16]; /// Name of this section.
    char segname[16]; /// Segment this section goes in.
    uint addr; /// Memory address of this section.
    uint size; /// Size in bytes of this section.
    uint offset; /// File offset of this section.
    uint align_; /// Section alignment (power of 2).
    uint reloff; /// File offset of relocation entries.
    uint nreloc; /// Number of relocation entries.
    uint flags; /// Flags (section type and attributes).
    uint reserved1; /// Reserved (for offset or index).
    uint reserved2; /// Reserved (for count or sizeof).
} section;


typedef struct section_64 { /* for 64-bit architectures */
    char sectname[16];          /* name of this section */
    char segname[16];           /* segment this section goes in */
    uint64_t addr;              /* memory address of this section */
    uint64_t size;              /* size in bytes of this section */
    uint32_t offset;            /* file offset of this section */
    uint32_t align;             /* section alignment (power of 2) */
    uint32_t reloff;            /* file offset of relocation entries */
    uint32_t nreloc;            /* number of relocation entries */
    uint32_t flags;             /* flags (section type and attributes)*/
    uint32_t reserved1;         /* reserved (for offset or index) */ 
    uint32_t reserved2;         /* reserved (for count or sizeof) */ 
    uint32_t reserved3;         /* reserved */
} section_64;


#endif 
