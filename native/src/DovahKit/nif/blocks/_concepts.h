#pragma once
#include <concepts>

namespace nifDK {
   template<typename T> concept block_type_has_name = requires {
      { T::type_name } -> std::same_as<const char* const&>;
   };
}