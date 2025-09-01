# Baseer Makefile

CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -ldl

# Files
# CORE = main.c baseer_core.c
CORE = main.c
EXT1 = ./extensions/print_version.c
EXT2 = ./extensions/print_size.c
EXT3 = ./extensions/print_hex.c
EXT4 = ./extensions/print_string.c

TARGET = baseer
EXT1_SO = ./extensions/print_version.so
EXT2_SO = ./extensions/print_size.so
EXT3_SO = ./extensions/print_hex.so
EXT4_SO = ./extensions/print_string.so


# Default target: build executable and extensions
# all: $(TARGET) $(EXT1_SO) $(EXT2_SO)
all: $(TARGET) $(EXT1_SO) $(EXT2_SO) $(EXT3_SO) $(EXT4_SO)

# Build core executable
$(TARGET): $(CORE) baseer.h
	$(CC) $(CFLAGS) $(CORE) -o $(TARGET) $(LDFLAGS)


# $(TARGET): $(CORE) baseer.h
# 	$(CC) $(CORE) -o $(TARGET)

# Build dynamic extension 1
$(EXT1_SO): $(EXT1) baseer.h
	$(CC) $(CFLAGS) -shared $(EXT1) -o $(EXT1_SO)

# Build dynamic extension 2
$(EXT2_SO): $(EXT2) baseer.h
	$(CC) $(CFLAGS) -shared $(EXT2) -o $(EXT2_SO)


$(EXT3_SO): $(EXT3) baseer.h
	$(CC) $(CFLAGS) -shared $(EXT3) -o $(EXT3_SO)

$(EXT4_SO): $(EXT4) baseer.h
	$(CC) $(CFLAGS) -shared $(EXT4) -o $(EXT4_SO)

# Clean generated files
clean:
	rm -f $(TARGET) $(EXT1_SO) $(EXT2_SO) $(EXT3_SO) $(EXT4_SO)
	# rm -f $(TARGET) 


# gcc -fPIC -shared print_ext.c -o print_ext.so
