#include "./edge_link.h"
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
   using cls          = wrappers::navmesh_edge_link;
   using form_type    = dovah::loaded_forms::Navmesh;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::navmesh_edge_link)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->geometry.edge_links;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }

   namespace _getters {
      int parent_navmesh(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_edge_link wrapper has no underlying object (deleted?)");
         return push_native_object(self.stub);
      }
      int target_navmesh(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_edge_link wrapper has no underlying object (deleted?)");
         return push_native_object(wrapped->navmesh);
      }
      int target_triangle(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<form_type>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_edge_link wrapper has no underlying object (deleted?)");
         if (!wrapped->navmesh) {
            lua_pushnil(L);
            return 1;
         }
         
         // return self.target_navmesh.triangles[self.triangle_index]
         wrapper target_mesh;
         target_mesh.stub = wrapped->navmesh.get_form_stub();
         target_mesh.type = wrapper_type::form;
         int count_pushed = core::subsystems::userdata::get().push(L, target_mesh, wrappers::navmesh_triangle::metatable_key);
         if (count_pushed == 0) {
            lua_pushnil(L);
            return 1;
         } else if (count_pushed > 1) {
            lua_pop(L, count_pushed);
            lua_pushnil(L);
            return 1;
         }
         lua_getfield(L, -1, "triangles");
         lua_remove(L, -2); // remove form from the stack
         lua_geti(L, -1, wrapped->triangle);
         return 1;
      }
      int target_triangle_index(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_edge_link wrapper has no underlying object (deleted?)");
         lua_pushnumber(L, wrapped->triangle);
         return 1;
      }
      int type(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_edge_link wrapper has no underlying object (deleted?)");
         switch (wrapped->type) {
            case wrapped_type::type::portal:
               lua_pushstring(L, "portal");
               break;
            case wrapped_type::type::ledge_up:
               lua_pushstring(L, "ledge up");
               break;
            case wrapped_type::type::ledge_down:
               lua_pushstring(L, "ledge down");
               break;
            case wrapped_type::type::enable_disable_portal:
               lua_pushstring(L, "enable/disable portal");
               break;
            default:
               lua_pushnumber(L, (int)wrapped->type);
               break;
         }
         return 1;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "parent_navmesh",        &_getters::parent_navmesh },
      { "target_navmesh",        &_getters::target_navmesh },
      { "target_triangle",       &_getters::target_triangle },
      { "target_triangle_index", &_getters::target_triangle_index },
      { "type",                  &_getters::type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}