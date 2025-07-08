#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map {
   class precomputed_path_is_empty final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr precomputed_path_is_empty(
            form_stub& subject,
            size_t which
         )
         :
            base_form_load_warning(subject),
            which(which)
         {}

         size_t which; // the N-th path in this file
   };
}
#include "../../../_util.undef.h"