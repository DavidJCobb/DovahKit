#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::note {
   //
   // A non-texture note contained an XNAM subrecord for a texture path.
   //
   class non_texture_note_includes_texture_path : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr non_texture_note_includes_texture_path(form_stub& subject) : base_form_load_warning(subject) {}
   };
}
#include "../../../_util.undef.h"