#pragma once
#include "../tools/_all.h"

namespace dovahkit::subsystems::worldedit::tools {
   using all_tools_with_options = all_tools::filter_types<[]<typename T>() { return requires { typename T::options; }; }>;
}