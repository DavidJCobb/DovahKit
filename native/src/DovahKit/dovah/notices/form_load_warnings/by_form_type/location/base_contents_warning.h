#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::location {
   class base_contents_warning : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class contents_type {
            enable_parent,
            exterior_cell_list,
            initially_disabled_ref,
            persist_location_ref,
            special_ref,
            unique_actor,
         };

      public:
         using base_form_load_warning::base_form_load_warning;
   };
}
#include "../../../_util.undef.h"