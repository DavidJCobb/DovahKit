#pragma once

namespace dovahkit::subsystems::worldinput {
   enum class optional_yn {
      unspecified,
      no,
      yes,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput::optional_yn> {
   using value_type = dovahkit::subsystems::worldinput::optional_yn;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ unspecified, no, yes };
   }();
};