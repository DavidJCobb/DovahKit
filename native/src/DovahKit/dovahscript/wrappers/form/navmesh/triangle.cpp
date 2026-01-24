#include "./triangle.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Navmesh.h"
#include "../navmesh.h"
#include "./collection_triangle_cover_edges.h"
#include "./vertex.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh_triangle;
   using form_type    = dovah::loaded_forms::Navmesh;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::navmesh_triangle)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->geometry.triangles;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }

   namespace _getters {
      template<auto Flag>
      int _flag_getter(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, !!(wrapped->flags & Flag));
         return 1;
      }

      int cover_edges(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle wrapper has no underlying object (deleted?)");
         wrapper out = self;
         out.append_part(wrapper_part_types::navmesh_triangle_cover_edge);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::navmesh_triangle_cover_edges.registry_key);
      }

      int cover_is_auto_generated(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_triangle wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, wrapped->cover.is_auto_generated());
         return 1;
      }

      // TODO: When we eventually implement write-access, this should return a collection, not a new table
      int vertex_indices(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<form_type>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_door_link wrapper has no underlying object (deleted?)");
         lua_newtable(L);
         lua_pushnumber(L, wrapped->vertices[0]);
         lua_seti(L, -2, 1);
         lua_pushnumber(L, wrapped->vertices[1]);
         lua_seti(L, -2, 2);
         lua_pushnumber(L, wrapped->vertices[2]);
         lua_seti(L, -2, 3);
         return 1;
      }

      // TODO: When we eventually implement write-access, this should return a collection, not a new table
      int vertices(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<form_type>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "navmesh_door_link wrapper has no underlying object (deleted?)");

         auto _push_vertex = [L, form](size_t i) {
            if (i >= form->geometry.vertices.size()) {
               lua_pushnil(L);
               return;
            }
            wrapper out;
            out.stub = &form->stub;
            out.type = wrapper_type::form;
            out.append_part(wrapper_part_types::navmesh_vertex);
            out.is_collection = true;
            out.into_collection(i);
            int count_pushed = core::subsystems::userdata::get().push(L, out, wrappers::navmesh_triangle::metatable_key);
            if (count_pushed == 0) {
               lua_pushnil(L);
            } else if (count_pushed > 1) {
               lua_pop(L, count_pushed - 1);
            }
         };

         lua_newtable(L);
         _push_vertex(wrapped->vertices[0]);
         lua_seti(L, -2, 1);
         _push_vertex(wrapped->vertices[1]);
         lua_seti(L, -2, 2);
         _push_vertex(wrapped->vertices[2]);
         lua_seti(L, -2, 3);
         return 1;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "cover_edges",             &_getters::cover_edges },
      { "cover_is_auto_generated", &_getters::cover_is_auto_generated },
      { "deleted",                 &_getters::_flag_getter<wrapped_type::flag::deleted> },
      { "found",                   &_getters::_flag_getter<wrapped_type::flag::found> },
      { "has_load_door",           &_getters::_flag_getter<wrapped_type::flag::has_load_door> },
      { "no_large_creatures",      &_getters::_flag_getter<wrapped_type::flag::no_large_creatures> },
      { "overlapping",             &_getters::_flag_getter<wrapped_type::flag::overlapping> },
      { "preferred",               &_getters::_flag_getter<wrapped_type::flag::preferred> },
      { "vertex_indices",          &_getters::vertex_indices },
      { "vertices",                &_getters::vertices },
      { "water",                   &_getters::_flag_getter<wrapped_type::flag::is_water> },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::navmesh_triangle_cover_edges);
   }
}