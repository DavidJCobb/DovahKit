#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::story_manager_node {
   //
   // Something edited the hardcoded Root node and gave it a parent.
   //
   class root_is_not_the_root final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr root_is_not_the_root(
            form_stub& subject
         )
         :
            base_form_load_warning(subject)
         {}
   };
}
#include "../../../_util.undef.h"