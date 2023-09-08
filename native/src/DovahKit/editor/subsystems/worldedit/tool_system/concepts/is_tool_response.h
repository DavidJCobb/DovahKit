#pragma once
#include "../utils/all_tool_response_types.h"
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T> concept is_tool_response = all_tool_response_types::contains_type<T>;
}