#include "./bitwriter.h"
#include "./bitreader.h"

namespace {
   using namespace cobb::streams;

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

   #ifndef __INTELLISENSE__ // ugh
   static_assert(
      ([]() -> bool {
         std::string  a_src("Hello, world!");
         std::wstring b_src(L"Hello, world!");

         bitwriter writer;
         writer.write(a_src);
         writer.write(b_src);

         std::string  a_dst;
         std::wstring b_dst;

         bitreader reader;
         reader.set_buffer(writer.data(), writer.size());
         reader.read(a_dst);
         reader.read(b_dst);

         if (a_src != a_dst)
            return false;
         if (b_src != b_dst)
            return false;

         return true;
      })(),
      "String test."
   );
   #endif

   static_assert(
      []() -> bool {
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
      }(),
      "Read/write member function test."
   );
}