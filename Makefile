# Baseer Makefile

CC      = gcc
CFLAGS  = -Wall -fPIC
LDFLAGS = -ldl

# Source Files
CORE            = main.c baseer.c
DEFAULT         = modules/default/bx_default.c
BX_BINHEAD      = modules/binhead/bx_binhead.c
BPARSER         = modules/bparser/bparser.c
BX_ELF          = modules/bx_elf/bx_elf.c
B_ELF_METADATA  = modules/b_elf_metadata/b_elf_metadata.c
B_DEBUG         = modules/b_debugger/debugger.c
BX_TAR         = modules/bx_tar/bx_tar.c

# Build directories
BUILDDIR        = build
MODULEDIR       = $(BUILDDIR)/modules

# Targets
TARGET          = $(BUILDDIR)/baseer
BX_BINHEAD_SO   = $(MODULEDIR)/bx_binhead.so
BPARSER_SO      = $(MODULEDIR)/bparser.so
BX_ELF_SO       = $(MODULEDIR)/bx_elf.so
B_ELF_METADATA_SO = $(MODULEDIR)/b_elf_metadata.so
B_DEBUG_SO      = $(MODULEDIR)/b_debugger.so
BX_TAR_SO         = $(MODULEDIR)/bx_tar.so

# Default target
all: $(TARGET) $(BX_BINHEAD_SO) $(BPARSER_SO) $(BX_ELF_SO) $(B_ELF_METADATA_SO) $(B_DEBUG_SO) $(B_TAR_SO)

# Ensure build directories exist
$(BUILDDIR) $(MODULEDIR):
	mkdir -p $@

# Core executable
$(TARGET): $(CORE) $(DEFAULT) $(BX_BINHEAD) $(BPARSER) $(BX_ELF) $(B_ELF_METADATA) $(B_DEBUG) $(B_TAR) baseer.h | $(BUILDDIR)
	$(CC) $(CORE) $(DEFAULT) $(BPARSER) $(BX_BINHEAD) $(BX_ELF) $(B_ELF_METADATA) $(B_DEBUG)  $(BX_TAR) -ludis86 -o $@

# Shared libraries
$(BX_BINHEAD_SO): $(BX_BINHEAD) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(BPARSER_SO): $(BPARSER) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(BX_ELF_SO): $(BX_ELF) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(B_ELF_METADATA_SO): $(B_ELF_METADATA) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared $< -o $@

$(B_DEBUG_SO): $(B_DEBUG) | $(MODULEDIR)
	$(CC) $(CFLAGS) -shared -ludis86 $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) *.so

