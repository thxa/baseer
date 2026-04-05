# Changelog

All notable changes to Baseer will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [0.3.0] - Unreleased

### Added
- **Test suite**: Unit tests for hashmap, baseer core, and bparser modules using a custom single-header test framework (`tests/`)
- **CI pipeline**: GitHub Actions workflow with both Release and Debug+Sanitizer builds (`.github/workflows/ci.yml`)
- **Logging system**: Centralized leveled logging (ERROR/WARN/INFO/DEBUG) via `utils/b_log.h`, controllable with `BASEER_LOG_LEVEL` environment variable
- **Configurable decompiler path**: RetDec binary location can now be overridden via `BASEER_RETDEC_BIN` environment variable
- **Mach-O build support**: Added Mach-O module sources and headers to CMakeLists.txt (was missing, causing linker errors)
- **ELF input validation**: Bounds checking for section headers, program headers, string table indices, and section body reads against file size

### Fixed
- **Critical**: `memset(ctx, 0, sizeof(ctx))` in debugger only cleared 8 bytes (pointer size) instead of full `context` struct -- now uses `sizeof(context)`
- **Critical**: `destroy_bp_sym()` used wrong loop variable (`ptr` instead of `ptr1`), causing symbol memory to never be freed
- **Critical**: `handle_action()` had no return value on fallthrough path -- undefined behavior, now returns `false`
- **Memory leak**: `parse_cmd()` leaked `line` buffer when process had exited (early return without `free(cmd)`)
- **Memory leak**: CLI `args` command called `strdup()` without ever freeing previous arguments; now frees old args before overwriting
- **Memory leak**: CLI `free(line)` was only called in the final `else` branch; now called on all `continue` paths
- **Null deref**: `vmmap` command could dereference NULL if `/proc/pid/maps` failed to open; now checks `fopen` return
- **Buffer overflow**: Replaced `sprintf` with `snprintf` in debugger for `/proc/pid/maps` path construction

### Changed
- **Security**: Replaced `system()` call in decompiler module with `fork()`/`execl()` to avoid shell injection risks
- **Code organization**: Moved static arrays (`cmds[]`, `flags[]`, `regs_64[]`, `regs_32[]`) from `debugger.h` to `debugger.c` to prevent duplicate symbols across translation units

### Removed
- Large blocks of commented-out code in `bx_elf.c`, `debugger.c`, `bx_binhead.c`, `b_elf_metadata.c`, and `main.c`

### Spelling Fixes
- "Closeing" -> "Closing" in `baseer.c`
- "faild" -> "failed" in `debugger.c`

## [0.2.0] - 2025

### Added
- Mach-O format support (metadata parsing)
- Interactive CLI with tab completion
- Decompiler integration (RetDec)
- ptrace-based debugger with breakpoints and register inspection
- TAR archive metadata parsing

## [0.1.0] - 2025

### Added
- Initial release
- ELF metadata, disassembly, and section/symbol parsing
- Modular callback-tree architecture
- Memory and streaming file access modes
