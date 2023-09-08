#pragma once
#include "../tools/_all.h"
#include "./all_tools_with_responses.h"

namespace dovahkit::subsystems::worldedit::tools {
   namespace impl {
      template<typename T> struct tool_to_tool_response_type {
         using type = typename T::response;
      };
   }
   using all_tool_response_types = all_tools_with_responses::map_types<impl::tool_to_tool_response_type>; // list of all response classes from tools that have them
}