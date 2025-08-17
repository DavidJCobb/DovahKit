#pragma once
#include <array>
#include "../form_types.h"

namespace dovah {
   // Every form type that can be placed in an inventory. This differs from 
   // all_carryable_form_types in that it doesn't include "pseudo-items" 
   // such as leveled lists.
   constexpr const auto all_item_form_types = std::array{
      form_type::ammo,
      form_type::apparatus,
      form_type::armor,
      form_type::book,
      form_type::formlist,
      form_type::ingredient,
      form_type::key,
      form_type::light, // light forms can be flagged as "carryable;" this is how torches work
      form_type::misc_item,
      form_type::note,
      form_type::potion,
      form_type::scroll,
      form_type::soul_gem,
      form_type::weapon,
   };
}