#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::actor_value_info {
   class unterminated_perk_tree_node final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unterminated_perk_tree_node(form_stub& subject, form_stub* perk) : base_form_load_warning(subject), perk(perk) {}

         form_stub* perk;
   };
}
#include "../../../_util.undef.h"