// retdec_mem_wrapper.cpp
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <retdec/retdec.h>   // RetDec C++ API headers

extern "C" {

// Takes a pointer to ELF data in memory and its size.
// Returns heap-allocated C string with decompiled code, or NULL on error.
// Caller must free() the returned pointer.
char* decompile_elf_from_memory(bparser* parser, void *arg) {
    try {
        // Configure RetDec
        retdec::Settings settings;
        // Optional: settings.setArchitecture("x86"); // or detect from ELF header
        
        // TODO: set settings for decompilation as needed

        retdec::Decompiler decompiler(settings);

        // Copy ELF data into a vector<uint8_t>
        const uint8_t* bytes = static_cast<const uint8_t*>(parser->source.mem.data);
        const uint8_t size = parser->source.mem.size;
        std::vector<uint8_t> buffer(bytes, bytes + size);

        // Start decompilation from memory buffer
        auto decompilation = decompiler.startDecompilationFromBuffer(buffer);

        // Wait until finished
        decompilation->waitUntilFinished();

        // Get the C-like output
        std::string output = decompilation->getOutputHll();

        // Allocate C string
        char* result = (char*)std::malloc(output.size() + 1);
        if (!result) return NULL;
        std::memcpy(result, output.c_str(), output.size() + 1);

        return result;
    }
    catch (...) {
        return NULL;
    }
}

} // extern "C"
