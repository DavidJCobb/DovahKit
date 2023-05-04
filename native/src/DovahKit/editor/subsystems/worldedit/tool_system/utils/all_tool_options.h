#pragma once
#include "../tools/_all.h"
#include "./all_tools_with_options.h"

namespace dovahkit::subsystems::worldedit::tools {
   namespace impl {
      template<typename T> struct tool_to_tool_options {
         using type = typename T::options;
      };
   }
   using all_tool_options = all_tools_with_options::map_types<impl::tool_to_tool_options>; // list of all options classes from tools that have them
}