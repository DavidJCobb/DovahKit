#pragma once
#include <type_traits>
#include "dovah/localization/category.h"
#include "dovah/types/unmanaged_localized_string_use.h" // `loaded_game_setting` manages l-string use info itself
#include "lu/type_containers/type_to_value_map.h"

namespace dovah::facts::game_settings {
   using value_types = lu::type_containers::type_to_value_map<
      lu::type_containers::type_to_value_map_pair<bool,    'b'>,
      lu::type_containers::type_to_value_map_pair<float,   'f'>,
      lu::type_containers::type_to_value_map_pair<int32_t, 'i'>,
      lu::type_containers::type_to_value_map_pair<unmanaged_localized_string_use<localization::category::general>, 's'>
   >;
}