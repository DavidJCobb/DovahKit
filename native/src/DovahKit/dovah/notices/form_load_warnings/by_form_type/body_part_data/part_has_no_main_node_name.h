#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::body_part_data {
   class part_has_no_main_node_name : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr part_has_no_main_node_name(
            form_stub& stub,
            size_t  which_part
         )
         :
            base_form_load_warning(stub),
            which_part(which_part)
         {}

         size_t which_part; // relative only to those parts that were not discarded during load
   };
}
#include "../../../_util.undef.h"