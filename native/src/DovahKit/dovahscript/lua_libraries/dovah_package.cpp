#include "dovah.h"
#include <cassert>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/setfuncs.h"
#include "../core/subsystems/coordinator.h"
#include "../core/subsystems/permissions.h"
#include "../push_native_object.h"
#include "../send_script_task.h"

#include "../api_helpers/load_resource.h"

namespace {
   using namespace dovahscript;
   
   namespace _definitions {
      int load_file(lua_State* L) {
         auto params = api_helpers::pull_load_resource_params(L, 1);
         params.load_from = decltype(params)::source::script_package;
         return api_helpers::load_and_push_resource(L, params);
      }
   }
   
   const std::initializer_list<luaL_Reg> _functions = {
      luaL_Reg{ "load_file", &_definitions::load_file },
   };
}

namespace dovahscript::lua_libraries {
   namespace dovah_package {
      extern void import(lua_State* L) {
         auto& coordinator_s = core::subsystems::coordinator::get();
         if (coordinator_s.package_folder_path().isEmpty())
            return;
         //
         int type = lua_getglobal(L, "dovah");
         assert(type == LUA_TTABLE && "the 'dovah' library needs to be imported before the 'dovah.package' library!");
         //
         lua_createtable(L, 0, _functions.size());
         cobb::lua::setfuncs(L, _functions);
         //
         lua_setfield(L, -2, "package");
         lua_pop(L, 1);
      }
   }
}