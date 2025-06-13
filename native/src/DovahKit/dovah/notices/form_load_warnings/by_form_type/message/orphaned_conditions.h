#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::message {
   class orphaned_conditions : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr orphaned_conditions(
            form_stub& stub,
            size_t count,
            bool   retained
         )
         :
            base_form_load_warning(stub),
            count(count),
            retained(retained)
         {}

         size_t count;
         bool   retained; // whether we retained the conditions, creating a blank button for them
   };
}
#include "../../../_util.undef.h"