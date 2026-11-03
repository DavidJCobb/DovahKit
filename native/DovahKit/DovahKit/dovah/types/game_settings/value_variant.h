#pragma once
#include <variant>
#include "dovah/facts/game_setting_types.h"

namespace dovah::types::game_settings {
   using value_variant = facts::game_setting_types::unpack_types_into<std::variant>;
}
