#include "./is_form_type_legal_to_create.h"
#include "editor/core.h"

namespace editor_helpers {
   extern bool is_form_type_legal_to_create(dovah::form_type ft) {
      switch (ft) {
         case dovah::form_type::none:
         case dovah::form_type::file_header:
         case dovah::form_type::file_record_group:
            //
            // These form types either aren't real forms, or can only be used by 
            // hardcoded forms (e.g. the PapyrusPersistenceForm is "none"-typed).
            //
            return false;
         case dovah::form_type::actor_value_info:
            //
            // Fallout 4 introduced the ability to create user-defined AVIFs. However, 
            // this isn't possible in Skyrim for a number of reasons.
            //
            return false;
      }
      const auto& info = dovah::form_type_info::lookup(ft);
      if (info.flags & dovah::form_type_info::flag::is_singleton)
         return false;
      if (info.flags & dovah::form_type_info::flag::is_skyrim_special) {
         const auto current_game = DovahKitCore::get().get_current_game();
         if (current_game != dovah::game::skyrim_special)
            return false;
      }
      return true;
   }
}
