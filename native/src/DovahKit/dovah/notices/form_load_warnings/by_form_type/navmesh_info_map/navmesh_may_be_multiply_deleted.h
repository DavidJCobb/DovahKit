#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map {
   class navmesh_may_be_multiply_deleted final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr navmesh_may_be_multiply_deleted(
            form_stub& subject,
            form_stub& navmesh
         )
         :
            base_form_load_warning(subject),
            navmesh(navmesh)
         {}

         form_stub& navmesh;
   };
}
#include "../../../_util.undef.h"