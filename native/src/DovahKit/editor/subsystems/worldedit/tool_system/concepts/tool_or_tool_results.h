#pragma once
#include "../tools/_all.h"
#include "./is_tool_results.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T> concept tool_or_tool_results = all_tools::contains_type<T> || is_tool_results<T>;
}