#pragma once
#include "../tools/_all.h"
#include "../utils/all_tool_options.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Options> concept is_tool_options = all_tool_options::contains_type<Options>;
}