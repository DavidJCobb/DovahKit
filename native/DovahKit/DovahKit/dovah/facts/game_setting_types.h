#pragma once
#include <type_traits>
#include "dovah/types/localized_string_type.h"
#include "dovah/types/managed_localized_string_use.h"
#include "dovah/types/unmanaged_localized_string_use.h"
#include "lu/type_containers/type_to_value_map.h"

namespace dovah::facts {
   template<bool Managed>
   using game_setting_types = lu::type_containers::type_to_value_map<
      lu::type_containers::type_to_value_map_pair<bool,    'b'>,
      lu::type_containers::type_to_value_map_pair<float,   'f'>,
      lu::type_containers::type_to_value_map_pair<int32_t, 'i'>,
      lu::type_containers::type_to_value_map_pair<
         std::conditional_t<
            Managed,
            managed_localized_string_use<localized_string_type::general>,
            unmanaged_localized_string_use<localized_string_type::general>
         >,
         's'
      >
   >;
}