#pragma once
#include "../tools/_all.h"
#include "../utils/all_tool_response_types.h"
#include "../utils/all_tools_with_responses.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T> concept tool_response_or_tool_with_response = (tools::all_tools_with_responses::contains_type<T> || tools::is_tool_response<T>);
}