#include "./vertex.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Navmesh.h"
#include "../navmesh.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh_vertex;
   using form_type    = dovah::loaded_forms::Navmesh;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::navmesh_vertex)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->geometry.vertices;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }

   namespace _getters {
      int x(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (!wrapped)
            return 0;
         lua_pushnumber(L, wrapped->x);
         return 1;
      }
      int y(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (!wrapped)
            return 0;
         lua_pushnumber(L, wrapped->y);
         return 1;
      }
      int z(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (!wrapped)
            return 0;
         lua_pushnumber(L, wrapped->z);
         return 1;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
      { "z", &_getters::z },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}