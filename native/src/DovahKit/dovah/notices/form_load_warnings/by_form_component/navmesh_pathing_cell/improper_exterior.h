#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::navmesh_pathing_cell {
   class improper_exterior : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr improper_exterior(
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