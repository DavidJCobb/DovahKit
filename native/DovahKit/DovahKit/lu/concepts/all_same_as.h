#pragma once
#include <type_traits>

namespace lu::concepts {
   template<typename Desired, typename... Types>
   concept all_same_as = (std::is_same_v<Desired, Types> && ...);
}
