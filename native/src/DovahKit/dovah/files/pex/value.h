#pragma once
#include <string>
#include <variant>
#include "./tabled_string.h"
#include "./underlying_value_type.h"

namespace dovah::pex {
   struct value { // "Variable Data" on UESP
      underlying_value_type type = underlying_value_type::none; // needed to distinguish between object names (std::string) and string literals (std::string)
      //
      std::variant<
         std::monostate,
         bool,
         float,
         int32_t,
         tabled_string
      > content;

      constexpr bool is_of_type(underlying_value_type t) const noexcept;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./value.inl"