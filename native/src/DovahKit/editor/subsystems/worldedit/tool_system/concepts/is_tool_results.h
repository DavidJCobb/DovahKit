#pragma once
#include "../utils/all_tool_results.h"
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Results> concept is_tool_results = all_tool_results::contains_type<Results>;
}