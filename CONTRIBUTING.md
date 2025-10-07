# Contributing to Baseer

Thank you for your interest in contributing to **Baseer**!
This guide explains how to add new features, extend file format support, and follow project standards.

---

## Understanding Baseer’s Extension System

Baseer operates using **callbacks** tied to **magic numbers**.
Each supported file format (like ELF or TAR) has a specific magic number that triggers a handler function when detected.

When Baseer opens a file:

1. It reads the header.
2. Matches it against entries in the `bmagic` array.
3. Executes the corresponding parser callback (`bx_<format>`).

Each `bx_<format>` function can apply multiple tool callbacks (like metadata, disassembler, or debugger).

---

## Adding a New File Format

Follow these steps to add support for a new file type (e.g., PDF, PE, PNG, ...):

### 1. Create a New Module

Inside the `modules/` directory, create a folder for your format:

```
modules/pdf/
```

Then add a new file:

```
modules/pdf/bx_pdf.c
```

### 2. Define the Format Callback

Your callback should follow the same pattern as other formats (e.g., `bx_elf`):

```c
bool bx_pdf(bparser* parser, void* arg)
{
    int argc = *((inputs*)arg)->argc;
    char** args = ((inputs*)arg)->args;

    for (int i = 2; i < argc; i++) {
        if (strcmp("-m", args[i]) == 0)
            bparser_apply(parser, b_pdf_metadata, arg);
        else if (strcmp("-c", args[i]) == 0)
            bparser_apply(parser, b_pdf_content, arg);
        else
            fprintf(stderr, "[!] Unsupported flag: %s\n", args[i]);
    }

    return true;
}
```

### 3. Implement Tool Functions

Create tool callbacks for specific operations:

```c
bool b_pdf_metadata(bparser* parser, void* arg)
{
    printf("PDF metadata reader not implemented yet.\n");
    return true;
}

bool b_pdf_content(bparser* parser, void* arg)
{
    printf("PDF content extractor not implemented yet.\n");
    return true;
}
```

### 4. Register Your Format

Edit the `magics` array (usually in `baseer.c`):

```c
bmagic magics[] = {
    {"ELF", ELF_MAGIC, reverse_bytes(ELF_MAGIC), bx_elf, 0},
    {"TAR", TAR_MAGIC, reverse_bytes(TAR_MAGIC), bx_tar, 257},
    {"PDF", PDF_MAGIC, reverse_bytes(PDF_MAGIC), bx_pdf, 0},
    {NULL, 0, 0, NULL, 0}
};
```

### 5. Rebuild

```bash
make clean
make
```

### 6. Test Your Format

Run Baseer with your new format:

```bash
./baseer sample.pdf -m
```

---

## Naming Conventions

| Type            | Example              | Description                          |
| --------------- | -------------------- | ------------------------------------ |
| Format callback | `bx_<format>`        | Main entry for file format           |
| Tool callback   | `b_<tool>`           | Function used inside format callback |
| Header dump     | `dump_<format>hdr()` | Function for metadata output         |
| Debugger        | `b_debugger()`       | Core debugging tool                  |
| Decompiler      | `b_decompiler()`     | Decompiler module                    |

---

## Code Style Guidelines

* Use **C99** standard.
* Follow existing formatting and naming conventions.
* Keep functions small and modular.
* Always return `true` or `false` for callback success/failure.
* Use `fprintf(stderr, …)` for error messages.
* Keep console outputs colored using defined macros (e.g., `COLOR_RED`, `COLOR_GREEN`, `COLOR_BLUE`).

---

## Tips for New Developers

* Start by reading existing implementations in `modules/elf/` and `modules/tar/`.
* Understand how `bparser_apply()` dispatches functions.
* Use the **hashmap** inside `inputs` for sharing data between callbacks.
* Test your module with small, well-known files.
* If your format requires byte-order conversion, use `reverse_bytes()`.

---

## Future Contributions

You’re encouraged to contribute:

* New format handlers (Mach-O, PDF, PNG, ZIP, etc.)
* Additional tools (debuggers, extractors, analyzers)
* Improvements to the CLI and parser
* Documentation and tutorials

---

## Thanks

Your contributions make Baseer stronger and more flexible.
Feel free to open issues or pull requests anytime!

