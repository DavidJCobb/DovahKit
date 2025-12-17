#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::story_manager_node {
   class orphaned final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr orphaned(
            form_stub& subject
         )
         :
            base_form_load_warning(subject)
         {}
   };
}
#include "../../../_util.undef.h"