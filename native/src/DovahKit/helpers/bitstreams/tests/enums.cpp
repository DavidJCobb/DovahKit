#pragma once
#include "../reader.h"
#include "../writer.h"

#include <cstdint>
#include "../enum_serialization_options.h"

namespace cobb::tests::bitstreams {
   // explicit bitcount
   enum class test_enum_a {
      a,
      b,
      c,
   };

   // explicit bitcount
   // explicit allowed values
   enum class test_enum_b {
      a,
      b,
      c,
      d,
      e,
   };

   // explicit bitcount
   // explicit discontiguous allowed values
   enum class test_enum_c {
      a = 2,
      b = 4,
      c = 6,
      d = 8,
      e = 10,
   };

   // no explicit bitcount or underlying type
   enum class test_enum_d {
      a,
      b,
      c,
   };
}
template<> struct cobb::bitstreams::enum_serialization_options<cobb::tests::bitstreams::test_enum_a> {
   using value_type = cobb::tests::bitstreams::test_enum_a;

   static constexpr const size_t bitcount = 2;
};
template<> struct cobb::bitstreams::enum_serialization_options<cobb::tests::bitstreams::test_enum_b> {
   using value_type = cobb::tests::bitstreams::test_enum_b;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto valid_values = []() {
      using enum value_type;
      return std::array<value_type, 5>{ a, b, c, d, e };
   }();
};
template<> struct cobb::bitstreams::enum_serialization_options<cobb::tests::bitstreams::test_enum_c> {
   using value_type = cobb::tests::bitstreams::test_enum_c;

   static constexpr const size_t bitcount = 4;
   static constexpr const auto valid_values = []() {
      using enum value_type;
      return std::array<value_type, 5>{ a, b, c, d, e };
   }();
};

namespace {
   constexpr const size_t bitcount_of_data_header = []() {
      cobb::bitstreams::writer w;
      w.stream(w.header());

      return w.get_bitpos();
   }();

   using test_enum_a = cobb::tests::bitstreams::test_enum_a;
   using test_enum_b = cobb::tests::bitstreams::test_enum_b;
   using test_enum_c = cobb::tests::bitstreams::test_enum_c;
   using test_enum_d = cobb::tests::bitstreams::test_enum_d;

   constexpr const auto enum_bitcount_a = cobb::bitstreams::enum_serialization_options<test_enum_a>::bitcount;
   constexpr const auto enum_bitcount_b = cobb::bitstreams::enum_serialization_options<test_enum_b>::bitcount;
   constexpr const auto enum_bitcount_c = cobb::bitstreams::enum_serialization_options<test_enum_c>::bitcount;
}

namespace {
   constexpr bool result_01 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      test_enum_a src = test_enum_a::b;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      if (w.get_bitpos() - bitcount_of_data_header != enum_bitcount_a)
         return false;
      return true;
   }();

   constexpr bool result_02 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      test_enum_a src = test_enum_a::b;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      test_enum_a dst = {};

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream(dst);

      return r.get_bitpos() - bitcount_of_data_header == enum_bitcount_a;
   }();

   constexpr bool result_03 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      test_enum_a src = test_enum_a::b;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      test_enum_a dst = {};

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream(dst);

      return src == dst;
   }();

   static_assert(
      result_01,
      "Test for an enum with an explicitly-set bitcount: do we write the correct number of bits?"
   );
   static_assert(
      result_02,
      "Test for an enum with an explicitly-set bitcount: do we read the correct number of bits?"
   );
   static_assert(
      result_03,
      "Round-trip test for an enum with an explicitly-set bitcount."
   );
}

namespace {
   constexpr bool result_04 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      test_enum_b src = test_enum_b::b;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      test_enum_b dst = {};

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream(dst);

      return src == dst;
   }();
   constexpr bool result_05 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      test_enum_c src = test_enum_c::b;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      test_enum_c dst = {};

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream(dst);

      return src == dst;
   }();

   static_assert(
      result_04,
      "Round-trip test for an enum with an explicitly-set bitcount and a list of valid values."
   );
   static_assert(
      result_05,
      "Round-trip test for an enum with an explicitly-set bitcount and a non-contiguous list of valid values."
   );

   static_assert(
      cobb::bitstreams::util::enum_type_information<test_enum_b>::value_is_valid((test_enum_b)-7) == false,
      "Identifying invalid values (contiguous case)."
   );
   static_assert(
      cobb::bitstreams::util::enum_type_information<test_enum_c>::value_is_valid((test_enum_c)-7) == false,
      "Identifying invalid values (non-contiguous case)."
   );
}

namespace {
   /*// Un-commenting this will result in a compiler warning from this file, if everything's working properly.
   constexpr bool result_06 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      test_enum_d src = test_enum_d::b;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      return true;
   }();
   static_assert(
      result_06,
      "Round-trip test for an enum with no explicit overrides; the lack of any such overrides should trigger a warning."
   );
   //*/
}