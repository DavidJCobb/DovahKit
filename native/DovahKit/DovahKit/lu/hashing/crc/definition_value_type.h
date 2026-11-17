#pragma once
#include "../../type_aliases/uint_of_size.h"
#include "./definition.h"

namespace lu::hashing::crc {
   template<rocksoft_definition Definition>
      requires (Definition.bitcount > 0)
   using definition_value_type = uint_of_size<((Definition.bitcount / 8) + ((Definition.bitcount % 8) ? 1 : 0))>;
}
