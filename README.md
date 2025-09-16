# بصير (Baseer)

**بصير (Baseer)** is a flexible, C-based reverse engineering analysis tool built on a **Core + Extensions** architecture. It can handle files from **any programming language** and allows you to open files, extract byte blocks, and analyze them using modular, customizable extensions.

> ⚠️ Note: This project is still under development and may change frequently.

---

## Architecture
![Architecture](base.png) 

### 1. Core

The Core handles the essential operations:

- **Open File**: Read files in raw byte format.
- **Extract Byte Blocks**: load block of bytes into memory.
- **Close File**: Release resources when done.
- **Handle Extensions**: Load and manage various extensions.
- **Manage Pipeline**: Pass byte blocks through extensions sequentially or in parallel.

### 2. Extensions

Extensions add advanced capabilities:

- **Fix Header**: Repair file headers or order.
- **Identify Block Type**: Detect byte block type or metadata.
- **Disassembler**: Convert binaries to assembly instructions.
- **Decompiler**: Convert binaries to readable source code.
- **Debugger**: Dynamically monitor and analyze files.

---
[docs](./docs/html)


<<<<<<< HEAD
## Libraries Used
- **udis86**: Used for disassembling x86 and x64 architecture binaries.  
  [GitHub Repository](https://github.com/vmt/udis86)



## Install Baseer 

### Installation on Arch Linux
[Baseer on AUR](https://aur.archlinux.org/packages/baseer)

You can install **Baseer** from the AUR using an AUR helper like `yay`:
```bash
yay -S baseer
```
> Make sure you have an AUR helper installed (e.g., `yay`, `paru`) before running the command.

### Uninstallation
To remove Baseer, use:
```bash
pacman -Rs baseer
```

### Install from Source
To install **Baseer** from source:
```bash
make install
```

To uninstall:
```bash
make uninstall
```
=======

## Requirements
- udis86

## Install Baseer 
```bash 
make install
```


>>>>>>> 9c46ae5 (new changes)

## Usage

1. Compile the Core (C-based):

```bash
make
```


2. Run Baseer

Analyze a file using one of the following modes:
- Show metadata:
```bash
baseer <file> -m
```
- Disassemble:
```bash 
./baseer <file> -a
```

- Launch debugger:
```bash
./baseer <file> -d
```


<!-- 2. Run the Core and specify the file: -->
<!-- - Show metadata of a file -->
<!-- ./baseer <file> -m -->

<!-- # Disassemble a file -->
<!-- ./baseer <file> -a -->

<!-- # Launch debugger for a file -->
<!-- ./baseer <file> -d -->


<!-- 3. Enable desired extensions: -->
<!-- ```bash -->
<!-- baseer sample.bin --extensions fix_header identify_block disassembler -->
<!-- ``` -->


<!-- 4. Use a pipeline of extensions: -->
<!-- ```bash -->
<!-- baseer sample.bin --pipeline fix_header|identify_block|disassembler -->
<!-- ``` -->

## Features

- Written in C for speed and low-level control.
- Language-agnostic: works with files from any programming language.
- Modular and extensible architecture.
<!-- - Supports complex analysis pipelines. -->
- Easy API for creating new extensions.

## Contributing

Create new extensions by inheriting from the Extension Base Class. Each extension should include:

- Extension name.
- Main function to analyze or modify byte blocks.
- Ability to integrate with the pipeline.

## Supported File Types (Magic Numbers)

Use this checklist to see which file types Baseer can currently handle:

- [x] **ELF** - `7F 45 4C 46` (Executable and Linkable Format)
- [x] **TAR** - `75 73 74 61 72` (TAR archive)
- [ ] **PDF** - `25 50 44 46` (Portable Document Format)
- [ ] **PNG** - `89 50 4E 47 0D 0A 1A 0A` (Portable Network Graphics)
- [ ] **JPEG** - `FF D8 FF` (JPEG image)
- [ ] **GIF** - `47 49 46 38` (Graphics Interchange Format)
- [ ] **ZIP** - `50 4B 03 04` (ZIP archive)
- [ ] **RAR** - `52 61 72 21 1A 07 00` (RAR archive)
- [ ] **7Z** - `37 7A BC AF 27 1C` (7-Zip archive)
- [ ] **EXE/DOS MZ** - `4D 5A` (Windows executable)
- [ ] **Mach-O** - `CF FA ED FE` (Mac OS X executable)
- [ ] **TIFF** - `49 49 2A 00` / `4D 4D 00 2A` (Tagged Image File Format)
- [ ] **MP3** - `49 44 33` (MP3 audio)
- [ ] **WAV** - `52 49 46 46` (Waveform Audio File)
- [ ] **BMP** - `42 4D` (Bitmap image)
- [ ] **ISO** - `43 44 30 30 31` (ISO 9660 CD-ROM image)
- [ ] **GZIP** - `1F 8B` (GZIP compressed)
- [ ] **FLAC** - `66 4C 61 43` (Free Lossless Audio Codec)
- [ ] **MIDI** - `4D 54 68 64` (MIDI sound file)
- [ ] **Microsoft Office (OLE)** - `D0 CF 11 E0 A1 B1 1A E1` (Word, Excel, PowerPoint)
- [ ] **Android APK** - `50 4B 03 04` (ZIP-based APK archive)
- [ ] **VMware Disk (VMDK)** - `4B 44 4D` (Virtual Machine Disk)
- [ ] **WebAssembly** - `00 61 73 6D` (WASM binary)
- [ ] **SQLite** - `53 51 4C 69 74 65 20 66 69 6C 65` (SQLite database)
- [ ] **XZ** - `FD 37 7A 58 5A 00` (XZ compressed)
- [ ] **CAB** - `4D 53 43 46` (Microsoft Cabinet file)
- [ ] **LZ4** - `04 22 4D 18` (LZ4 Frame Format)

> This checklist is based on the [Wikipedia list of file signatures](https://en.wikipedia.org/wiki/List_of_file_signatures). You can extend Baseer to support more types in the future.

