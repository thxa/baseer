cmake_minimum_required(VERSION 3.10)
project(baseer C)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC")

# Installation paths
set(CMAKE_INSTALL_PREFIX "/opt/baseer" CACHE PATH "Installation prefix")
set(BINDIR "${CMAKE_INSTALL_PREFIX}")
set(LIBDIR "${CMAKE_INSTALL_PREFIX}/modules")

# Include directories
include_directories(
    libs/libudis86
    libs/linenoise
    libs/macho
)

# Udis86 sources
set(UDIS86_SRC
    libs/libudis86/decode.c
    libs/libudis86/itab.c
    libs/libudis86/syn.c
    libs/libudis86/syn-att.c
    libs/libudis86/syn-intel.c
    libs/libudis86/udis86.c
)

# Core sources
set(CORE_SRC
    main.c
    baseer.c
    utils/ui.c
    utils/b_CLI.c
    libs/linenoise/linenoise.c
)

# Module sources
set(DEFAULT_SRC modules/default/bx_default.c)
set(BX_BINHEAD_SRC modules/binhead/bx_binhead.c)
set(BPARSER_SRC modules/bparser/bparser.c)
set(B_HASHMAP_SRC modules/b_hashmap/b_hashmap.c)
set(BX_ELF_SRC modules/bx_elf/bx_elf.c)
set(BX_ELF_UTILS_SRC modules/bx_elf_utils/bx_elf_utils.c)
set(B_ELF_METADATA_SRC modules/b_elf_metadata/b_elf_metadata.c)
set(BX_ELF_DISASM_SRC modules/bx_elf_disasm/bx_elf_disasm.c)
set(B_DEBUG_SRC modules/b_debugger/debugger.c)
set(BX_TAR_SRC modules/bx_tar/bx_tar.c)
set(BX_deElf_SRC modules/bx_deElf/bx_deElf.c)
set(BX_MACHO_SRC modules/bx_macho/bx_macho.c)
set(BX_MACHO_UTILS_SRC modules/bx_macho_utils/bx_macho_utils.c)
set(B_MACHO_METADATA_SRC modules/b_macho_metadata/b_macho_metadata.c)
set(BX_MACHO_DISASM_SRC modules/bx_macho_disasm/bx_macho_disasm.c)
set(BX_PDF_SRC modules/bx_pdf/bx_pdf.c)
set(B_PDF_METADATA_SRC modules/b_pdf_metadata/b_pdf_metadata.c)

# Main executable
add_executable(baseer
    ${CORE_SRC}
    ${DEFAULT_SRC}
    ${BPARSER_SRC}
    ${B_HASHMAP_SRC}
    ${BX_BINHEAD_SRC}
    ${BX_ELF_SRC}
    ${BX_ELF_UTILS_SRC}
    ${B_ELF_METADATA_SRC}
    ${B_DEBUG_SRC}
    ${BX_TAR_SRC}
    ${BX_deElf_SRC}
    ${BX_ELF_DISASM_SRC}
    ${BX_MACHO_SRC}
    ${BX_MACHO_UTILS_SRC}
    ${B_MACHO_METADATA_SRC}
    ${BX_MACHO_DISASM_SRC}
    ${BX_PDF_SRC}
    ${B_PDF_METADATA_SRC}
    ${UDIS86_SRC}
)

target_link_libraries(baseer dl)

# Shared library modules
add_library(bx_binhead SHARED ${BX_BINHEAD_SRC})
add_library(bparser SHARED ${BPARSER_SRC})
add_library(b_hashmap SHARED ${B_HASHMAP_SRC})
add_library(bx_elf SHARED ${BX_ELF_SRC})
add_library(b_elf_metadata SHARED ${B_ELF_METADATA_SRC})
add_library(bx_tar SHARED ${BX_TAR_SRC})
add_library(bx_deElf SHARED ${BX_deElf_SRC})

# Modules that need udis86
add_library(b_debugger SHARED ${B_DEBUG_SRC} ${UDIS86_SRC})
add_library(bx_elf_disasm SHARED ${BX_ELF_DISASM_SRC} ${UDIS86_SRC})
add_library(bx_pdf SHARED ${BX_PDF_SRC})
add_library(b_pdf_metadata SHARED ${B_PDF_METADATA_SRC})

# Set output directory for modules
set_target_properties(
    bx_binhead bparser b_hashmap bx_elf b_elf_metadata
    b_debugger bx_tar bx_deElf bx_elf_disasm
    bx_pdf b_pdf_metadata
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
)

# Installation rules
install(TARGETS baseer DESTINATION ${BINDIR})
install(TARGETS 
    bx_binhead bparser b_hashmap bx_elf b_elf_metadata
    b_debugger bx_tar bx_deElf bx_elf_disasm
    bx_pdf b_pdf_metadata
    LIBRARY DESTINATION ${LIBDIR}
)
install(FILES README.md LICENSE DESTINATION ${BINDIR})
install(PROGRAMS install_decompiler.sh DESTINATION ${BINDIR})

# Create symlink (custom command)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${BINDIR}/baseer /usr/bin/baseer)")

# ========================= Tests =========================
enable_testing()

# Test: hashmap
add_executable(test_hashmap
    tests/test_hashmap.c
    ${B_HASHMAP_SRC}
)
add_test(NAME test_hashmap COMMAND test_hashmap WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Test: baseer core
add_executable(test_baseer_core
    tests/test_baseer_core.c
    baseer.c
    libs/linenoise/linenoise.c
    ${B_HASHMAP_SRC}
)
target_include_directories(test_baseer_core PRIVATE libs/linenoise)
add_test(NAME test_baseer_core COMMAND test_baseer_core WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Test: bparser
add_executable(test_bparser
    tests/test_bparser.c
    modules/bparser/bparser.c
    baseer.c
    libs/linenoise/linenoise.c
    ${B_HASHMAP_SRC}
)
target_include_directories(test_bparser PRIVATE libs/linenoise)
add_test(NAME test_bparser COMMAND test_bparser WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Test: Mach-O
add_executable(test_macho
    tests/test_macho.c
    modules/bparser/bparser.c
    baseer.c
    libs/linenoise/linenoise.c
    ${B_HASHMAP_SRC}
)
target_include_directories(test_macho PRIVATE libs/linenoise)
add_test(NAME test_macho COMMAND test_macho WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Test: PDF
add_executable(test_pdf
    tests/test_pdf.c
    modules/bparser/bparser.c
    baseer.c
    libs/linenoise/linenoise.c
    ${B_HASHMAP_SRC}
)
target_include_directories(test_pdf PRIVATE libs/linenoise)
add_test(NAME test_pdf COMMAND test_pdf WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Custom target to run all tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_hashmap test_baseer_core test_bparser test_macho test_pdf
    COMMENT "Running all tests"
)

# Uninstall target
add_custom_target(uninstall
    COMMAND rm -rf ${BINDIR}
    COMMAND rm -rf ${LIBDIR}
    COMMAND rm -f /usr/bin/baseer
    COMMENT "Uninstalling baseer"
)
