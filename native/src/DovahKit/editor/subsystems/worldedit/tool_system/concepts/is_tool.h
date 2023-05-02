#pragma once
#include <type_traits>
#include "../tools/_base.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T> concept is_tool = std::is_base_of_v<_base, T>;
}