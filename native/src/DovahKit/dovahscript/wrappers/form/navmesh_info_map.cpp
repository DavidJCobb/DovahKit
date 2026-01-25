#include "./navmesh_info_map.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/NavMeshInfoMap.h"
#include "./navmesh_info_map/collection_precomputed_paths.h"
#include "./navmesh_info_map/navmesh_info_collection.h"
#include "./navmesh_info_map/precomputed_path.h"

#include "dovahscript/lua_classes/vector3.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh_info_map;
   using wrapped_type = cls::wrapped_type;

   namespace _getters {
      int debug_copy_as_table(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;

         lua_newtable(L);
         {
            lua_newtable(L);
            for (const auto& pair : form->navmesh_infos.infos) {
               if (!push_native_object(pair.first))
                  continue;
               const auto& src = pair.second;
               lua_newtable(L);
               {
                  if (push_native_object(src.navmesh)) {
                     lua_setfield(L, -2, "navmesh");
                  }
                  lua_pushnumber(L, src.category);
                  lua_setfield(L, -2, "category");
                  {
                     lua_newtable(L);
                     for (size_t i = 0; i < src.unk08.size(); ++i) {
                        lua_pushnumber(L, src.unk08[i]);
                        lua_seti(L, -2, i + 1);
                     }
                     lua_setfield(L, -2, "unk08");
                  }
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
               }
               lua_rawset(L, -3);
            }
            lua_setfield(L, -2, "navmesh_infos");
         }
         {
            auto& list = form->precomputed_paths.paths;
            lua_newtable(L);
            for (size_t i = 0; i < list.size(); ++i) {
               const auto& path = list[i];
               lua_newtable(L);
               {
                  size_t k = 0;
                  for (size_t j = 0; j < path.navmeshes.size(); ++j)
                     if (push_native_object(path.navmeshes[j]))
                        lua_seti(L, -2, ++k); // precomputed_paths[i][k] = ...
               }
               lua_seti(L, -2, i + 1); // precomputed_paths[i] = ...
            }
            lua_setfield(L, -2, "precomputed_paths");
         }
         {
            const auto& list = form->road_markers.raw_entries();
            lua_newtable(L);
            for (const auto& item : list) {
               if (!item.navmesh)
                  continue;
               if (push_native_object(item.navmesh)) {
                  lua_pushnumber(L, item.index);
                  lua_rawset(L, -3);
               }
            }
            lua_setfield(L, -2, "road_markers");
         }
         {
            std::set<dovah::form_stub*> deleted;
            for (const auto& use : form->deleted_navmeshes.masters)
               deleted.insert(use.get_form_stub());
            for (const auto& use : form->deleted_navmeshes.active_file)
               deleted.insert(use.get_form_stub());
            lua_newtable(L);
            {
               size_t i = 0;
               for (dovah::form_stub* item : deleted) {
                  if (!item)
                     continue;
                  if (push_native_object(item))
                     lua_seti(L, -2, ++i);
               }
            }
            lua_setfield(L, -2, "deleted_navmeshes");
         }
         return 1;
      }

      int navmesh_infos(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::navmesh_info_collection);
         return core::subsystems::userdata::get().push(L, out, wrappers::navmesh_info_collection::metatable_key);
      }
      int precomputed_paths(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::navi_precomputed_path);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::navmesh_info_map_precomputed_paths.registry_key);
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "debug_copy_as_table", &_getters::debug_copy_as_table },
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "navmesh_infos",     &_getters::navmesh_infos },
      { "precomputed_paths", &_getters::precomputed_paths },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::navmesh_info_map_precomputed_paths);
      define_collection_metatable(L, collections::navmesh_info_map_precomputed_path_nodes);
   }
}
#pragma endregion