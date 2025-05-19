#pragma once
#include "../tools/_base.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Tool> concept tool_with_options = tool_with_options_member_type<Tool>;
}