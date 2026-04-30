#pragma once
#include <concepts>

namespace dovahscript::api_helpers::native_lists::impl::fields::initialize_value {
   template<typename Spec>
   concept present = requires {
      { Spec::initialize_value };
   };
   template<typename Spec>
   concept valid = requires {
      requires present<Spec>;
      { Spec::initialize_value() } -> std::same_as<typename Spec::value_working_type>;
   };
}