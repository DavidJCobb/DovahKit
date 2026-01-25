#include "./navmesh_info_collection.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/NavMeshInfoMap.h"
#include "../navmesh_info_map.h"

#include "dovahscript/lua_classes/vector3.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh_info_collection;
   using form_type    = dovah::loaded_forms::NavMeshInfoMap;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::navmesh_info_collection)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      return &form->navmesh_infos;
   }

   namespace _methods {
      int get(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (!wrapped)
            cobb::lua::error(L, "navmesh_info_collection wrapper has no underlying object (deleted?)");
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::navmesh);
         if (!value)
            cobb::lua::error(L, "nil is not allowed here");

         auto it = wrapped->infos.find(value);
         if (it == wrapped->infos.end()) {
            lua_pushnil(L);
            return 1;
         }
         
         //
         // A fatal flaw in Dovahscript's wrapper design: we can only delve into collections 
         // using an eight-CC and an index. Wrappers can't expose items in a key/value map, 
         // if arbitrary non-integer keys are possible.
         // 
         // At the time of this writing, I'm only implementing NAVI accessors so I can use 
         // Lua scripts to verify that I'm loading NAVI properly, so I only need read access. 
         // Copying the navmesh info object into a table shall suffice.
         //

         const auto& src = it->second;

         lua_newtable(L);
         auto si_retval = lua_gettop(L);

         if (push_native_object(src.navmesh)) {
            lua_setfield(L, -2, "navmesh");
         }
         lua_pushnumber(L, src.category);
         lua_setfield(L, -2, "category");
         lua_classes::vector3::push_new_instance(L, src.approx_location.x, src.approx_location.y, src.approx_location.z);
         lua_setfield(L, -2, "approx_location");
         lua_pushnumber(L, src.preference);
         lua_setfield(L, -2, "preference");
         {
            lua_newtable(L); // links
            {
               lua_newtable(L);
               size_t i = 0;
               for (auto& use : src.links.edges)
                  if (push_native_object(use))
                     lua_seti(L, -2, ++i);
               lua_setfield(L, -2, "edges");
            }
            {
               lua_newtable(L);
               size_t i = 0;
               for (auto& use : src.links.preferred_edges)
                  if (push_native_object(use))
                     lua_seti(L, -2, ++i);
               lua_setfield(L, -2, "preferred_edges");
            }
            {
               lua_newtable(L);
               size_t i = 0;
               for (auto& door_link : src.links.doors)
                  if (push_native_object(door_link.door))
                     lua_seti(L, -2, ++i);
               lua_setfield(L, -2, "doors");
            }
            lua_setfield(L, -2, "links");
         }
         if (src.island.has_value()) {
            lua_newtable(L);
            {
               auto& island = src.island.value();
               lua_classes::vector3::push_new_instance(L, island.min.x, island.min.y, island.min.z);
               lua_setfield(L, -2, "min");
               lua_classes::vector3::push_new_instance(L, island.max.x, island.max.y, island.max.z);
               lua_setfield(L, -2, "max");
               {
                  lua_newtable(L);
                  for (size_t i = 0; i < island.triangles.size(); ++i) {
                     lua_newtable(L);
                     {
                        lua_pushnumber(L, island.triangles[i].vertices[0]);
                        lua_seti(L, -2, 1);
                        lua_pushnumber(L, island.triangles[i].vertices[1]);
                        lua_seti(L, -2, 2);
                        lua_pushnumber(L, island.triangles[i].vertices[2]);
                        lua_seti(L, -2, 3);
                     }
                     lua_seti(L, -2, i + 1);
                  }
                  lua_setfield(L, -2, "triangles");
               }
               {
                  lua_newtable(L);
                  for (size_t i = 0; i < island.vertices.size(); ++i) {
                     auto& v = island.vertices[i];
                     lua_classes::vector3::push_new_instance(L, v.x, v.y, v.z);
                     lua_seti(L, -2, i + 1);
                  }
                  lua_setfield(L, -2, "vertices");
               }
            }
            lua_setfield(L, -2, "island");
         }
         {
            using navmesh_pathing_cell = decltype(src.pathing_cell);
            if (std::holds_alternative<navmesh_pathing_cell::pathing_cell_exterior>(src.pathing_cell.data)) {
               auto& data = std::get<navmesh_pathing_cell::pathing_cell_exterior>(src.pathing_cell.data);
               lua_newtable(L);
               {
                  if (push_native_object(data.parent_world))
                     lua_setfield(L, -2, "worldspace");
                  lua_pushnumber(L, data.grid_x);
                  lua_setfield(L, -2, "grid_x");
                  lua_pushnumber(L, data.grid_y);
                  lua_setfield(L, -2, "grid_y");
               }
               lua_setfield(L, -2, "pathing_cell");
            } else if (std::holds_alternative<navmesh_pathing_cell::pathing_cell_interior>(src.pathing_cell.data)) {
               auto& data = std::get<navmesh_pathing_cell::pathing_cell_interior>(src.pathing_cell.data);
               lua_newtable(L);
               {
                  if (push_native_object(data.cell))
                     lua_setfield(L, -2, "cell");
               }
               lua_setfield(L, -2, "pathing_cell");
            }
         }

         lua_settop(L, si_retval);
         return 1;
      }
   }
   namespace _getters {
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "get",  &_methods::get },
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}