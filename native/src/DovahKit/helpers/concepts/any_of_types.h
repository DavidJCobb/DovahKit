#pragma once
#include <type_traits>

namespace cobb::concepts {
   template<typename T, typename... Candidates>
   concept any_of_types = (std::is_same_v<T, Candidates> || ...);
}
