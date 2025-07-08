#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map {
   class precomputed_path_has_gaps final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr precomputed_path_has_gaps(
            form_stub& subject,
            size_t which,
            form_stub* endpoint_a,
            bool endpoint_a_is_actually_at_an_end,
            form_stub* endpoint_b,
            bool endpoint_b_is_actually_at_an_end
         )
         :
            base_form_load_warning(subject),
            which(which),
            endpoints({
               .a = endpoint_a,
               .b = endpoint_b,
               .a_is_end = endpoint_a_is_actually_at_an_end,
               .b_is_end = endpoint_b_is_actually_at_an_end,
            })
         {}

         size_t which; // the N-th path in this file
         struct {
            form_stub* a;
            form_stub* b;
            bool a_is_end;
            bool b_is_end;
         } endpoints;
   };
}
#include "../../../_util.undef.h"