#include "./navmesh.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Navmesh.h"
#include "./navmesh/door_link.h"
#include "./navmesh/edge_link.h"
#include "./navmesh/triangle.h"
#include "./navmesh/vertex.h"
#include "./navmesh/collection_door_links.h"
#include "./navmesh/collection_edge_links.h"
#include "./navmesh/collection_triangles.h"
#include "./navmesh/collection_vertices.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::navmesh;
   using wrapped_type = cls::wrapped_type;

   namespace _getters {
      int door_links(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::navmesh_door_link);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::navmesh_door_links.registry_key);
      }
      int edge_links(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::navmesh_edge_link);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::navmesh_edge_links.registry_key);
      }
      int triangles(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::navmesh_triangle);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::navmesh_triangles.registry_key);
      }
      int version(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->geometry.version);
         return 1;
      }
      int vertices(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::navmesh_vertex);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::navmesh_vertices.registry_key);
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "door_links", &_getters::door_links },
      { "edge_links", &_getters::edge_links },
      { "triangles",  &_getters::triangles },
      { "version",    &_getters::version },
      { "vertices",   &_getters::vertices },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::navmesh_door_links);
      define_collection_metatable(L, collections::navmesh_edge_links);
      define_collection_metatable(L, collections::navmesh_triangles);
      define_collection_metatable(L, collections::navmesh_vertices);
   }
}
#pragma endregion