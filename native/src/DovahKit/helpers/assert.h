#pragma once
#include <string>

//
// USAGE: 
// #undef NDEBUG before including.
// Use the cobb__assert(condition, format_string, ...) macro.
//

namespace cobb {
   #if UNICODE
      using assert_string_t = std::wstring;
   #else
      using assert_string_t = std::string;
   #endif
   extern void _assert(const char* file, unsigned line, const assert_string_t format, ...);
}
#ifdef NDEBUG
   #define cobb__assert(condition, ...) (void(0))
#else
   #if UNICODE
      #define cobb__assert(condition, ...) (void)(!!(condition) || (cobb::_assert(__FILE__, (unsigned)(__LINE__), __VA_ARGS__),0) )
   #else
      #define cobb__assert(condition, ...) (void)(!!(condition) || (cobb::_assert(__FILE__, (unsigned)(__LINE__), __VA_ARGS__),0) )
   #endif
#endif