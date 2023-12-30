#include "ref_alias.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/form_stub.h"
#include "../../../../dovah/forms/Quest.h"
#include "../quest.h"

//
// MISSING APIS:
//  - Reference aliases
//     - Fill type and parameters
//     - Flags
//     - Added Factions
//     - Added Inventory
//     - Added Keywords
//     - Added Packages
//     - Added Spells
//     - Package override lists
//     - Additional voicetypes
//
#include "../../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for reference aliases is incomplete.");

namespace {
   using namespace dovahscript;
   using cls = wrappers::quest_ref_alias;
   
   namespace _getters {
      int display_name(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         return push_native_object(alias->display_name.get_form_stub());
      }
   }
   namespace _setters {
      int display_name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::message);
         self.before_edit();
         alias->display_name.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "display_name", &_getters::display_name },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "display_name", &_setters::display_name },
   };

   /*static*/ cls::wrapped_type* quest_ref_alias::unwrap(wrapper& w) {
      auto* alias = quest_alias::unwrap(w);
      if (alias && alias->type != dovah::loaded_forms::Alias::alias_type::reference)
         return nullptr;
      return (wrapped_type*)alias;
   }
}