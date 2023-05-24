#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class optional_yn {
      unspecified,
      no,
      yes,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldinput2::optional_yn> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldinput2::optional_yn::yes>;

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::optional_yn> {
   using value_type = dovahkit::subsystems::worldinput2::optional_yn;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ unspecified, no, yes };
   }();
};