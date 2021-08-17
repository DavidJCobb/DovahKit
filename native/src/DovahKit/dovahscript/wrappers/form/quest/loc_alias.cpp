#include "loc_alias.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../../../../dovah/form_stub.h"
#include "../../../../dovah/forms/Quest.h"
#include "../quest.h"
#include "../papyrus/root.h"

//
// MISSING APIS:
//  - Location aliases
//     - Fill type and parameters
//     - Flags
//
#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for location aliases is incomplete!")
#endif

namespace {
   using namespace dovahscript;
   using cls = wrappers::quest_loc_alias;
   
   namespace getters {
   }
   namespace setters {
   }
}

namespace dovahscript::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;

   /*static*/ cls::wrapped_type* quest_loc_alias::unwrap(wrapper& w) {
      auto* alias = quest_alias::unwrap(w);
      if (alias && alias->type != dovah::loaded_forms::Alias::alias_type::location)
         return nullptr;
      return (wrapped_type*)alias;
   }
}