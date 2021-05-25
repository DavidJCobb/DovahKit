#include "dds.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/lua_managed_resources.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/color.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::dds;

   namespace _methods {
   }
   namespace _getters {
   }
   namespace _setters {
   }
}

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ int cls::wrap_and_push(lua_State* L, LuaManagedResource& resource) {
      assert(resource.resource_type() == lua_managed_resource_type::dds);
      wrapper out;
      out.type = wrapper_type::lua_managed_resource;
      out.managed_resource = &resource;
      return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::metatable_key);
   }
}