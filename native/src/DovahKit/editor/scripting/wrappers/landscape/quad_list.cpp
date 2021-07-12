#include "quad_list.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../wrapper_util.h"

#include "../../../../dovah/forms/Landscape.h"
#include "quad.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::landscape_quad_list;

   namespace _getters {
      luastackchange_t top_left(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::top_left);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
      luastackchange_t top_right(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::top_right);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
      luastackchange_t bottom_left(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::bottom_left);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
      luastackchange_t bottom_right(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::bottom_right);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
   }
   namespace _setters {
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "top_left",     &_getters::top_left },
      { "top_right",    &_getters::top_right },
      { "bottom_left",  &_getters::bottom_left },
      { "bottom_right", &_getters::bottom_right },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;
}