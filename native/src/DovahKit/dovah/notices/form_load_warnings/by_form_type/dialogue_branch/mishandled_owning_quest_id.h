#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::dialogue_branch {
   class mishandled_owning_quest_id : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr mishandled_owning_quest_id(
            form_stub& subject,
            form_stub& intended_owning_quest
         )
         :
            base_form_load_warning(subject),
            intended_owning_quest(intended_owning_quest)
         {}

         form_stub& intended_owning_quest;
   };
}
#include "../../../_util.undef.h"