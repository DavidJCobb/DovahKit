#pragma once
#include <variant>
#include "dovah/facts/game_setting_types.h"

namespace dovah::types::game_settings {
   using managed_value_variant = facts::game_setting_types<true>::unpack_types_into<std::variant>;
}
