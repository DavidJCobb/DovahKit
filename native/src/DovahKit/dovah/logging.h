#pragma once
#include <cstdint>

namespace dovah {
   namespace logging {
      extern void print_line(const char* fmt, ...); // appends a newline
      extern void print(const char* fmt, ...); // does not append a newline
      extern const char* format_signature(uint32_t signature); // 'ABCD' -> "ABCD" // not thread-safe, and can only use one buffer at a time
      extern const char* format_signature(uint32_t signature, char out[5]); // 'ABCD' -> "ABCD" // thread-safe if out buffer is local to the thread

      extern const char* file_error_code_to_string(errno_t);
   }
}