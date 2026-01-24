#include "./triangle_cover_edge.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Navmesh.h"
#include "../navmesh.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh_triangle_cover_edge;
   using form_type    = dovah::loaded_forms::Navmesh;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::navmesh_triangle)
         return nullptr;
      if (w.parts[1].signature != wrapper_part_types::navmesh_triangle_cover_edge)
         return nullptr;
      if (w.parts[1].index < 0 || w.parts[1].index > 1)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->geometry.triangles;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i].cover;
   }

   wrapped_type::edge _edge_of(const wrapper& w) {
      return (wrapped_type::edge)w.parts[1].index;
   }

   namespace _getters {
      int covered_on_left(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle_cover_edge wrapper has no underlying object (deleted?)");
         lua_pushnumber(L, wrapped->is_covered_on_side(_edge_of(self), wrapped_type::side::left));
         return 1;
      }
      int covered_on_right(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle_cover_edge wrapper has no underlying object (deleted?)");
         lua_pushnumber(L, wrapped->is_covered_on_side(_edge_of(self), wrapped_type::side::right));
         return 1;
      }
      int height(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle_cover_edge wrapper has no underlying object (deleted?)");
         lua_pushnumber(L, wrapped->get_height(_edge_of(self)));
         return 1;
      }
      int type(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle_cover_edge wrapper has no underlying object (deleted?)");
         auto type = wrapped->get_type(_edge_of(self));
         switch (type) {
            case wrapped_type::type::covered_ledge:
               lua_pushstring(L, "covered ledge");
               break;
            case wrapped_type::type::covered_wall:
               lua_pushstring(L, "covered wall");
               break;
            case wrapped_type::type::uncovered_open_edge:
               lua_pushstring(L, "uncovered open edge");
               break;
            case wrapped_type::type::uncovered_wall:
               lua_pushstring(L, "uncovered wall");
               break;
            case wrapped_type::type::unused:
               lua_pushstring(L, "unused");
               break;
            default:
               lua_pushnumber(L, (int)type);
               break;
         }
         return 1;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "covered_on_left",  &_getters::covered_on_left },
      { "covered_on_right", &_getters::covered_on_right },
      { "height",           &_getters::height },
      { "type",             &_getters::type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}