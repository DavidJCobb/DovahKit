#pragma once
#include "../tools/_all.h"
#include "./all_tool_results.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Results> requires all_tool_results::contains_type<Results>
   using tool_for_results = all_tools_with_results::nth_type<
      all_tool_results::index_of_type<Results>
   >;
}
