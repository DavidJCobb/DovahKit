#pragma once
#include <string>
#include <variant>
#include "../type_containers/fixed_map.h"

namespace cobb::ini {
   using value_types = type_containers::fixed_map<
      type_containers::fixed_map_entry<bool,         'b'>,
      type_containers::fixed_map_entry<double,       'f'>,
      type_containers::fixed_map_entry<signed int,   'i'>,
      type_containers::fixed_map_entry<unsigned int, 'u'>,
      type_containers::fixed_map_entry<std::string,  's'> // UTF-8
   >;

   // I may change this into an untagged union in the future, since a setting name's first 
   // character acts as a tag anyway.
   using value_union = value_types::unpack_types_into<std::variant>;

   using value_variant = value_types::unpack_types_into<std::variant>;
}
