#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   class occlusion_box_should_be_a_plane : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr occlusion_box_should_be_a_plane(
            form_stub& stub
         )
         :
            base_form_load_warning(stub)
         {}
   };
}
#include "../../../_util.undef.h"