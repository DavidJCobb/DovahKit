#pragma once
#include "../tools/_all.h"
#include "../utils/all_tool_results.h"
#include "../utils/all_tools_with_results.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T> concept tool_results_or_tool_with_results = (tools::all_tools_with_results::contains_type<T> || tools::is_tool_results<T>);
}