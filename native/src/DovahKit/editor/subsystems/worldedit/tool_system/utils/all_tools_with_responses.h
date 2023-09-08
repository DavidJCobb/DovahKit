#pragma once
#include "../concepts/tool_with_response.h"
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   using all_tools_with_responses = all_tools::filter_types<[]<typename T>() { return tool_with_response<T>; }>;
}