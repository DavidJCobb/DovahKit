#pragma once
#include "../tools/_all.h"
#include "./is_tool_response.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T> concept tool_or_tool_response = all_tools::contains_type<T> || is_tool_response<T>;
}