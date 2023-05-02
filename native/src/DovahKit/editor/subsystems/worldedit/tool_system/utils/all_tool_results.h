#pragma once
#include "../tools/_all.h"
#include "./all_tools_with_results.h"

namespace dovahkit::subsystems::worldedit::tools {
   namespace impl {
      template<typename T> struct tool_to_tool_results {
         using type = typename T::results;
      };
   }
   using all_tool_results = all_tools_with_results::map_types<impl::tool_to_tool_results>; // list of all results classes from tools that have them
}