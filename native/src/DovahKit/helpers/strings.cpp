/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "strings.h"
#include <cstdarg>
#include <cstdint>
#include <cwctype>

namespace cobb {
   void sprintf(std::string& out, const char* format, ...) {
      va_list args;
      va_start(args, format);
      va_list safe;
      va_copy(safe, args);
      {
         char b[128];
         if (vsnprintf(b, sizeof(b), format, args) < 128) {
            out = b;
            va_end(safe);
            va_end(args);
            return;
         }
      }
      uint32_t s = 256;
      char* b = (char*)malloc(s);
      int32_t r = vsnprintf(b, s, format, args);
      while (r + 1 > s) {
         va_copy(args, safe);
         s += 20;
         free(b);
         b = (char*)malloc(s);
         r = vsnprintf(b, s, format, args);
      }
      out = b;
      free(b);
      va_end(safe);
      va_end(args);
   };
   void sprintfw(std::wstring& out, const wchar_t* format, ...) {
      va_list args;
      va_start(args, format);
      va_list safe;
      va_copy(safe, args);
      auto size = _vsnwprintf(nullptr, 0, format, args);
      out.resize(size + 1);
      size = _vsnwprintf(out.data(), size + 1, format, safe);
      out.resize(size);
      va_end(safe);
      va_end(args);
   };
   
   extern std::string format_string(const char* format, ...) {
      std::string out;
      //
      va_list args;
      va_start(args, format);
      va_list safe;
      va_copy(safe, args);
      {
         char b[128];
         if (vsnprintf(b, sizeof(b), format, args) < 128) {
            out = b;
            va_end(safe);
            va_end(args);
            return out;
         }
      }
      uint32_t s = 256;
      char* b = (char*)malloc(s);
      int32_t r = vsnprintf(b, s, format, args);
      while (r + 1 > s) {
         va_copy(args, safe);
         s += 20;
         free(b);
         b = (char*)malloc(s);
         r = vsnprintf(b, s, format, args);
      }
      out = b;
      free(b);
      va_end(safe);
      va_end(args);
      return out;
   }
   
   bool string_to_int(const char* str, int32_t& out, bool allowHexOrDecimal) {
      errno = 0;
      char* end = nullptr;
      uint32_t base = 10;
      if (allowHexOrDecimal) {
         const char* p = str;
         while (char c = *p) {
            if (isspace(c))
               continue;
            if (c == '0') {
               ++p;
               c = *p;
               if (c == 'x' || c == 'X') {
                  base = 16;
                  break;
               }
            }
            ++p;
         }
      }
      int32_t o = strtol(str, &end, base);
      if (end == str) // not a number
         return false;
      {  // if any non-whitespace chars after the found value, then the string isn't really a number
         char c = *end;
         while (c) {
            if (!isspace(c))
               return false;
            ++end;
            c = *end;
         }
      }
      if (errno == ERANGE) // out of range
         return false;
      out = o;
      return true;
   }
   bool string_to_int(const char* str, uint32_t& out, bool allowHexOrDecimal) {
      errno = 0;
      char* end = nullptr;
      uint32_t base = 10;
      if (allowHexOrDecimal) {
         const char* p = str;
         while (char c = *p) {
            if (isspace(c))
               continue;
            if (c == '0') {
               ++p;
               c = *p;
               if (c == 'x' || c == 'X') {
                  base = 16;
                  break;
               }
            }
            ++p;
         }
      }
      uint32_t o = strtoul(str, &end, base);
      if (end == str) // not a number
         return false;
      {  // if any non-whitespace chars after the found value, then the string isn't really a number
         char c = *end;
         while (c) {
            if (!isspace(c))
               return false;
            ++end;
            c = *end;
         }
      }
      if (errno == ERANGE) // out of range
         return false;
      out = o;
      return true;
   }
   bool string_to_float(const char* str, float& out) {
      errno = 0;
      char* end = nullptr;
      float o = strtof(str, &end);
      if (end == str) // not a number
         return false;
      {  // if any non-whitespace chars after the found value, then the string isn't really a number
         char c = *end;
         while (c) {
            if (!isspace(c))
               return false;
            ++end;
            c = *end;
         }
      }
      if (errno == ERANGE) // out of range
         return false;
      out = o;
      return true;
   }
   bool path_starts_with(const std::wstring& path, const std::wstring& prefix) {
      if (prefix.size() > path.size())
         return false;
      for (size_t i = 0; i < prefix.size(); i++) {
         wchar_t c = std::towlower(path[i]);
         wchar_t d = std::towlower(prefix[i]);
         if (c == d)
            continue;
         if (c == '/')
            if (d == '\\')
               continue;
            else
               return false;
         if (c == '\\')
            if (d == '/')
               continue;
            else
               return false;
         return false;
      }
      return true;
   }
   bool string_ends_with(const std::string& haystack, const std::string& suffix) noexcept {
      size_t h = haystack.size();
      size_t s = suffix.size();
      if (s > h)
         return false;
      return haystack.compare(h - s, std::string::npos, suffix, 0, std::string::npos) == 0;
   }

   std::string ltrim(std::string& subject) {
      return std::string(find_first_non_whitespace(subject), subject.cend());
   };
   std::string rtrim(std::string& subject) {
      return std::string(subject.cbegin(), find_last_non_whitespace(subject));
   };
   std::string trim(const std::string& subject) {
      return std::string(find_first_non_whitespace(subject), find_last_non_whitespace(subject));
   };
   std::wstring ltrim(std::wstring& subject) {
      return std::wstring(find_first_non_whitespace(subject), subject.cend());
   };
   std::wstring rtrim(std::wstring& subject) {
      return std::wstring(subject.cbegin(), find_last_non_whitespace(subject));
   };
   std::wstring trim(const std::wstring& subject) {
      return std::wstring(find_first_non_whitespace(subject), find_last_non_whitespace(subject));
   }
}