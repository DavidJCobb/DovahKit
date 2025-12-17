#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::story_manager_event_node {
   class unrecognized_event final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr unrecognized_event(
            form_stub& subject,
            uint32_t event
         )
         :
            base_form_load_warning(subject),
            event(event)
         {}

         uint32_t event;
   };
}
#include "../../../_util.undef.h"