#pragma once
#include <array>
#include <stdexcept>

namespace cobb {
   //
   // Compile-time string, suitable for use as a template parameter or (with some 
   // extra effort) a member of a templated structural class type.
   //
   template<size_t Size> // includes the null terminator
   class cs {
      public: // "structural class types" usable as template parameter types cannot have protected or private members >_>
         std::array<char, Size> _data = {};

      public:
         constexpr cs(const char(&c)[Size]) {
            for (size_t i = 0; i < Size; ++i)
               _data[i] = c[i];
         }

         constexpr const char* c_str() const noexcept { return _data.data(); }
         constexpr const char* data() const noexcept { return _data.data(); }
         constexpr size_t size() const noexcept { return Size - 1; }

         constexpr char operator[](std::size_t n) const {
            if (n >= Size)
               throw std::out_of_range("");
            return _data[n];
         }
   };

   template<size_t Size> cs(const char(&)[Size]) -> cs<Size>;
}