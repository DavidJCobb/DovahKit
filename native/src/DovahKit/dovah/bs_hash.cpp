#include "bs_hash.h"
#include <array>
#include <cassert>
#include <string>

//
// Hashing algorithm, as presented here, reverse-engineered from Oblivion and 
// checked against community code.
//

namespace {
   int32_t _hash_string(const char* string, uint8_t length) {
      if (length <= 0)
         return 0;
      int32_t result = 0;
      for (int i = 0; i < length; ++i) {
         result *= 0x1003F;
         //
         char c = string[i];
         if (c == '/')
            result += (int32_t)'\\';
         else
            result += tolower(c);
      }
      return result;
   }
   //
   std::array<std::string, 5> _extensions = {{
      "",
      ".NIF",
      ".KF",
      ".DDS",
      ".WAV"
   }};
}
namespace dovah {
   bs_hash::bs_hash(const char* name, const char* extension) {
      uint8_t length_b = strlen(name); // the length, truncated to a byte, is used frequently. some community code optimizes with (strnlen_s) but Oblivion itself truncates, not clamps, the length
      assert(length_b > 0 && "Oblivion's logic doesn't handle the case of hashing a zero-length string.");
      this->bytes[2] = length_b;
      this->bytes[3] = tolower((int32_t)name[0]);
      this->bytes[0] = tolower((int32_t)name[length_b - 1]);
      this->bytes[1] = (length_b > 2) ? tolower((int32_t)name[length_b - 2]) : 0;
      if (length_b > 3)
         this->dwords[1] = _hash_string(name + 1, length_b - 3);
      //
      if (!extension)
         return;
      this->dwords[1] += _hash_string(extension, strlen(extension));
      for (int i = 0; i < _extensions.size(); ++i) {
         auto& ext = _extensions[i];
         if (_stricmp(ext.c_str(), extension) == 0) {
            this->bytes[3] += uint8_t(i & ~3) << 5;
            this->bytes[0] += uint8_t(i & ~1) << 6;
            this->bytes[1] += uint8_t(i) << 7;
            return;
         }
      }
   }
}