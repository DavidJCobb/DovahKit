#pragma once
#include "../concepts/tool_with_results.h"
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   using all_tools_with_results = all_tools::filter_types<[]<typename T>() { return tool_with_results<T>; }>;
}