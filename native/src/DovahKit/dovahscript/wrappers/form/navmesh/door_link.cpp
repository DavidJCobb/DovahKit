#include "./door_link.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Navmesh.h"
#include "../navmesh.h"
#include "./triangle.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh_door_link;
   using form_type    = dovah::loaded_forms::Navmesh;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::navmesh_door_link)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->geometry.door_links;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }

   namespace _getters {
      int ref(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_door_link wrapper has no underlying object (deleted?)");
         return push_native_object(wrapped->door_ref);
      }
      int triangle(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<form_type>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_door_link wrapper has no underlying object (deleted?)");
         auto i = wrapped->triangle;
         if (i < 0 || i >= form->geometry.triangles.size()) {
            lua_pushnil(L);
            return 1;
         }

         wrapper out;
         out.stub = self.stub;
         out.type = wrapper_type::form;
         out.append_part(wrapper_part_types::navmesh_triangle);
         out.is_collection = true;
         out.into_collection(i);
         return core::subsystems::userdata::get().push(L, out, wrappers::navmesh_triangle::metatable_key);
      }
      int triangle_index(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_door_link wrapper has no underlying object (deleted?)");
         lua_pushnumber(L, wrapped->triangle);
         return 1;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "ref",            &_getters::ref },
      { "triangle",       &_getters::triangle },
      { "triangle_index", &_getters::triangle_index },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}