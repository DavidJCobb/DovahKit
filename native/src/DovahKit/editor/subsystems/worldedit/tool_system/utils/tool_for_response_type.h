#pragma once
#include "../tools/_all.h"
#include "./all_tool_response_types.h"
#include "./all_tools_with_responses.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Response> requires all_tool_response_types::contains_type<Response>
   using tool_for_response_type = all_tools_with_responses::nth_type<
      all_tool_response_types::index_of_type<Response>
   >;
}
