# Baseer Makefile

CC      = gcc
CFLAGS  = -Wall -fPIC
LDFLAGS = -ldl
CFLAGS += -Ilibs/libudis86 -Ilibs/linenoise 

# Source Files
CORE            = main.c baseer.c utils/ui.c utils/b_CLI.c libs/linenoise/linenoise.c
DEFAULT         = modules/default/bx_default.c
BX_BINHEAD      = modules/binhead/bx_binhead.c
BPARSER         = modules/bparser/bparser.c
B_HASHMAP 	= modules/b_hashmap/b_hashmap.c
BX_ELF          = modules/bx_elf/bx_elf.c
BX_ELF_UTILS    = modules/bx_elf_utils/bx_elf_utils.c
B_ELF_METADATA  = modules/b_elf_metadata/b_elf_metadata.c
BX_ELF_DISASM   = modules/bx_elf_disasm/bx_elf_disasm.c
B_DEBUG         = modules/b_debugger/debugger.c
BX_TAR          = modules/bx_tar/bx_tar.c
BX_deElf        = modules/bx_deElf/bx_deElf.c





# Udis86 sources
UDIS86_SRC = libs/libudis86/decode.c \
             libs/libudis86/itab.c \
             libs/libudis86/syn.c \
             libs/libudis86/syn-att.c \
             libs/libudis86/syn-intel.c \
             libs/libudis86/udis86.c


UDIS86_HDR = libs/libudis86/decode.h \
             libs/libudis86/itab.h \
             libs/libudis86/extern.h \
             libs/libudis86/syn.h \
             libs/libudis86/types.h \
             libs/libudis86/udint.h




# Build directories
BUILDDIR        = build
MODULEDIR       = $(BUILDDIR)/modules

# Install paths
PREFIX          = /usr/local
BINDIR          = $(PREFIX)/bin
LIBDIR          = $(PREFIX)/lib/baseer/modules

# Targets
TARGET          = $(BUILDDIR)/baseer
BX_BINHEAD_SO   = $(MODULEDIR)/bx_binhead.so
BPARSER_SO      = $(MODULEDIR)/bparser.so
B_HASHMAP_SO	= $(MODULEDIR)/b_hashmap.so
BX_ELF_SO       = $(MODULEDIR)/bx_elf.so
B_ELF_METADATA_SO = $(MODULEDIR)/b_elf_metadata.so
B_DEBUG_SO      = $(MODULEDIR)/b_debugger.so
BX_TAR_SO       = $(MODULEDIR)/bx_tar.so
BX_deElf_SO     = $(MODULEDIR)/bx_deElf.so
BX_ELF_DISASM_SO   = $(MODULEDIR)/bx_elf_disasm.so

# Default target
all: $(TARGET) $(BX_BINHEAD_SO) $(BPARSER_SO) $(BX_ELF_SO) $(B_ELF_METADATA_SO) $(B_DEBUG_SO) $(BX_TAR_SO) $(BX_deElf_SO) $(BX_ELF_DISASM_SO) $(B_HASHMAP_SO)

# Ensure build directories exist
$(BUILDDIR) $(MODULEDIR):
	mkdir -p $@

# Core executable
$(TARGET): $(CORE) $(DEFAULT) $(BX_BINHEAD) $(BPARSER) $(B_HASHMAP) $(BX_ELF) $(BX_ELF_UTILS) $(B_ELF_METADATA) $(B_DEBUG) $(BX_TAR) $(BX_deElf) baseer.h | $(BUILDDIR)

	$(CC) $(CFLAGS) $(CORE) $(DEFAULT) $(BPARSER) $(B_HASHMAP) $(BX_BINHEAD) $(BX_ELF) $(BX_ELF_UTILS) $(B_ELF_METADATA) $(B_DEBUG) $(BX_TAR) $(BX_deElf) $(BX_ELF_DISASM) $(UDIS86_SRC) -o $@
	# $(CC) $(CORE) $(DEFAULT) $(BPARSER) $(BX_BINHEAD) $(BX_ELF) $(B_ELF_METADATA) $(B_DEBUG) $(BX_TAR) $(BX_deElf) -ludis86 -o $@

# Shared libraries
$(BX_BINHEAD_SO): $(BX_BINHEAD) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(BPARSER_SO): $(BPARSER) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(B_HASHMAP_SO): $(B_HASHMAP) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(BX_ELF_SO): $(BX_ELF) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(B_ELF_METADATA_SO): $(B_ELF_METADATA) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@


$(B_DEBUG_SO): $(B_DEBUG) $(UDIS86_SRC) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $(B_DEBUG) $(UDIS86_SRC) -o $@

$(BX_ELF_DISASM_SO): $(BX_ELF_DISASM) $(UDIS86_SRC) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $(BX_ELF_DISASM) $(UDIS86_SRC) -o $@

# $(B_DEBUG_SO): $(B_DEBUG) | $(MODULEDIR)
# 	$(CC) $(CFLAGS) -shared -ludis86 $< -o $@

$(BX_TAR_SO): $(BX_TAR) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(BX_deElf_SO): $(BX_deElf) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@



# Install
install: all
	mkdir -p $(BINDIR) $(LIBDIR)
	cp $(TARGET) $(BINDIR)/
	cp $(MODULEDIR)/*.so $(LIBDIR)/

# Uninstall
uninstall:
	rm -f $(BINDIR)/baseer
	rm -rf $(LIBDIR)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) *.so

