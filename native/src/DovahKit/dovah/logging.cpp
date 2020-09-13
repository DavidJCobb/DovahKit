#include "logging.h"
#include <cstdarg>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace dovah {
   namespace logging {
      void print_line(const char* fmt, ...) {
         va_list args;
         va_start(args, fmt);
         vprintf(fmt, args);
         va_end(args);
         printf("\n");
      }
      void print(const char* fmt, ...) {
         va_list args;
         va_start(args, fmt);
         vprintf(fmt, args);
         va_end(args);
      }
      const char* format_signature(uint32_t signature) {
         static char buf[5];
         *(uint32_t*)buf = _byteswap_ulong(signature);
         buf[4] = '\0';
         return buf;
      }
      extern const char* format_signature(uint32_t signature, char out[5]) {
         *(uint32_t*)out = _byteswap_ulong(signature);
         out[4] = '\0';
         return out;
      }

      extern const char* file_error_code_to_string(errno_t code) {
         switch (code) {
            // Found a list of possible error codes for fopen: https://pubs.opengroup.org/onlinepubs/9699919799/functions/fopen.html
            case ENFILE:
               return "Too many files open (system-wide).";
            case EMFILE:
               return "Too many files open (this process).";
            case EINVAL:
               return "The file name or path may be invalid";
            case ELOOP:
               return "The file was inaccessible due to a cyclical reference among symbolic links in the file path.";
            case ENAMETOOLONG:
               return "The file was inaccessible; the path name (whether before or after symbolic links) is too long.";
            case EACCES:
               return "The file is locked, or you do not have permission to access it.";
            case EBUSY:
               return "The file is locked.";
            case ENOENT:
               return "The file does not exist.";
            case EROFS:
               return "The file exists on a read-only filesystem and cannot be opened for writing.";
            case ENOMEM:
               return "Insufficient memory.";
            case EISDIR:
               return "The 'file' is actually a directory and therefore cannot be opened for writing.";
            case ENOTDIR:
               return "The specified path is not a directory.";
         }
         return "";
      }
   }
}