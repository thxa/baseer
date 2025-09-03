# Baseer Makefile
 
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -ldl

# Files
# CORE = main.c baseer_core.c
CORE = main.c baseer.c
TARGET = baseer
DEFAULT = modules/default/bx_default.c
BX_BINHEAD = modules/binhead/bx_binhead.c
BPARSER = modules/bparser/bparser.c
BX_ELF = modules/bx_elf/bx_elf.c


# Default target: build executable and extensions
# all: $(TARGET) $(EXT1_SO) $(EXT2_SO)
all: $(TARGET) $(DEFAULT) $(BX_BINHEAD) $(BPARSER) $(BX_ELF )

# Build core executable
# $(TARGET): $(CORE) baseer.h
# 	$(CC) $(CFLAGS) $(CORE) -o $(TARGET) $(LDFLAGS)


$(TARGET): $(CORE) baseer.h $(DEFAULT) $(BX_BINHEAD) $(BPARSER) $(BX_ELF)
	$(CC) $(CORE) $(DEFAULT) $(BPARSER) $(BX_BINHEAD) $(BX_ELF) -o $(TARGET)

# gcc main.c baseer.c modules/default/mbx_default.c -o main 
# Build dynamic extension 1
# $(EXT1_SO): $(EXT1) baseer.h
# 	$(CC) $(CFLAGS) -shared $(EXT1) -o $(EXT1_SO)
# Clean generated files
clean:
	rm -f $(TARGET)


# gcc -fPIC -shared print_ext.c -o print_ext.so
