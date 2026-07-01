#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::body_part_data {
   class two_parts_have_the_same_main_node : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr two_parts_have_the_same_main_node(
            form_stub& stub,
            size_t part_a,
            size_t part_b
         )
         :
            base_form_load_warning(stub),
            part_a(part_a),
            part_b(part_b)
         {}

         size_t part_a; // relative only to those parts that were not discarded during load
         size_t part_b; // relative only to those parts that were not discarded during load
   };
}
#include "../../../_util.undef.h"