#include "quad_list.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../wrapper.h"

#include "quad.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::landscape_quad_list;

   namespace _getters {
      int top_left(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::top_left);
         return core::subsystems::userdata::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
      int top_right(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::top_right);
         return core::subsystems::userdata::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
      int bottom_left(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::bottom_left);
         return core::subsystems::userdata::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
      int bottom_right(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.into_collection(dovah::loaded_forms::Landscape::quad_indices::bottom_right);
         return core::subsystems::userdata::get().push(L, out, wrappers::landscape_quad::metatable_key);
      }
   }
   namespace _setters {
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "top_left",     &_getters::top_left },
      { "top_right",    &_getters::top_right },
      { "bottom_left",  &_getters::bottom_left },
      { "bottom_right", &_getters::bottom_right },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}