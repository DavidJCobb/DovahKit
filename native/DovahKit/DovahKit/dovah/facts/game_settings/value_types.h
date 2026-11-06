#pragma once
#include <type_traits>
#include "lu/type_containers/type_to_value_map.h"
#include "./string_id_type.h"

namespace dovah::facts::game_settings {
   using value_types = lu::type_containers::type_to_value_map<
      lu::type_containers::type_to_value_map_pair<bool,    'b'>,
      lu::type_containers::type_to_value_map_pair<float,   'f'>,
      lu::type_containers::type_to_value_map_pair<int32_t, 'i'>,
      lu::type_containers::type_to_value_map_pair<std::variant<std::string, string_id_type>, 's'>
   >;
}