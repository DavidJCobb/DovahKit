#include "./bitwriter.h"
#include "./bitreader.h"

using namespace cobb::streams;

namespace {
   static_assert(
      []() -> bool {
         bool v1 = false;
         int  v2 = 5;

         bitwriter writer;
         writer.write(v1);
         writer.write(v2);

         bitreader reader;
         reader.set_buffer(writer.data(), writer.size());
         
         reader.read(v1);
         if (v1 != false)
            return false;
         reader.read(v2);
         if (v2 != 5)
            return false;

         return true;
      }(),
      "Symmetric read/write test."
   );
}

namespace {
   //
   // Can't put the IIFE in the static_assert directly or else IntelliSense will 
   // throw a fit
   //
   constexpr auto string_test = []() -> bool {
      std::string  a_src("Hello, world!");
      std::wstring b_src(L"Hello, world!");

      bitwriter writer;
      writer.reserve(a_src.size() + 4);
      writer.write(a_src);
      writer.write(b_src);

      std::string  a_dst;
      std::wstring b_dst;

      bitreader reader;
      reader.set_buffer(writer.data(), writer.size());
      reader.read(a_dst);
      reader.read(b_dst);

      //
      // For whatever reason, IntelliSense projectile-vomits its guts out and dies 
      // if we use the built-in std::string::operator==
      //
      if (a_src.size() != a_dst.size())
         return false;
      for (size_t i = 0; i < a_src.size(); ++i) {
         if (a_src[i] != a_dst[i])
            return false;
      }

      // std::wstring::operator== works fine. What the hell?
      if (b_src != b_dst)
         return false;

      return true;
   }();

   static_assert(
      string_test,
      "String test."
   );
}

namespace {
   constexpr bool struct_test = []() -> bool {
      struct test_type {
         bool  v1 = {};
         int   v2 = {};
         float v3 = {};

         constexpr void read(bitreader& stream) {
            stream.read(v1, v2, v3);
         }
         constexpr void write(bitwriter& stream) const {
            stream.write(v1, v2, v3);
         }

         constexpr bool operator==(const test_type&) const noexcept = default;
      };

      test_type ref{ true, 5, 1.3 };

      bitwriter writer;
      writer.write(ref);

      bitreader reader;
      reader.set_buffer(writer.data(), writer.size());

      test_type retrieve;
      reader.read(retrieve);

      return (ref == retrieve);
   }();

   static_assert(
      struct_test,
      "Read/write member function test."
   );
}