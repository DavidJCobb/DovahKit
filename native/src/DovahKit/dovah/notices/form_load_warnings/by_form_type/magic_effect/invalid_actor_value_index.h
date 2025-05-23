#pragma once
#include <cstdint>
#include <limits>
#include "../../invalid_actor_value_index.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::magic_effect {
   class invalid_actor_value_index : public ::dovah::notices::form_load_warnings::invalid_actor_value_index {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class which_type {
            resist,
            assoc_item_1,
            assoc_item_2,
         };

      public:
         constexpr invalid_actor_value_index(form_stub& subject, int32_t av, which_type w)
         :
            ::dovah::notices::form_load_warnings::invalid_actor_value_index::invalid_actor_value_index(subject, av),
            which(w)
         {}

         which_type which;
   };
}
#include "../../../_util.undef.h"