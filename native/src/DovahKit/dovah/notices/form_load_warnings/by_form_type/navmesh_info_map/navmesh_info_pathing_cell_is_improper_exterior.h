#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map {
   class navmesh_info_pathing_cell_is_improper_exterior final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr navmesh_info_pathing_cell_is_improper_exterior(
            form_stub& subject,
            form_stub* navmesh,
            form_stub& cell
         )
         :
            base_form_load_warning(subject),
            navmesh(navmesh),
            cell(cell)
         {}

         form_stub* navmesh;
         form_stub& cell;
   };
}
#include "../../../_util.undef.h"