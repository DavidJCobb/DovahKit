#pragma once
#include <type_traits>
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Options> concept is_tool_options = cobb::tuples::contains_type_matching_functor<all_tools::as_tuple, []<typename Current>() -> bool {
      if constexpr (tool_with_options_member_type<Current>) {
         return std::is_same_v<Options, typename Current::options>;
      }
      return false;
   }>;
}