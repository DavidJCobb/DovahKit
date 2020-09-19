#include "assert.h"
#include <assert.h>
#include <cstdarg>
#include <stdio.h>
#include "strings.h"

namespace cobb {
   void _assert(const char* file, unsigned line, const assert_string_t format, ...) {
      #ifndef NDEBUG
         va_list args;
         va_start(args, format);
         va_list safe;
         va_copy(safe, args);
         //
         #if UNICODE
            constexpr auto _func = _vsnwprintf;
         #else
            constexpr auto _func = _vsnprintf;
         #endif
         //
         assert_string_t out;
         int needed = _func(nullptr, 0, format.c_str(), args);
         if (needed == -1) {
            out.clear();
         } else {
            size_t size = needed + 1;
            out.resize(size);
            needed = _func(out.data(), size, format.c_str(), args);
            if (needed == -1) {
               out.clear();
            } else {
               out.resize(needed);
            }
         }
         //
         #if UNICODE
            std::wstring widened_file(strlen(file), '\0');
            for (size_t i = 0; i < widened_file.size(); ++i)
               widened_file[i] = file[i];
            _wassert(out.c_str(), widened_file.c_str(), line);
         #else
            ::_assert(out.c_str(), file, line);
         #endif
      #endif
   }
}