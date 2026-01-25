#include "./precomputed_path.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/NavMeshInfoMap.h"
#include "../navmesh_info_map.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.navmesh.precomputed_path_nodes>";
}

namespace {
   using namespace dovahscript;

   using wrapped_type = dovah::loaded_forms::NavMeshInfoMap;

   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (self.parts[0].signature != wrapper_part_types::navi_precomputed_path)
         return 0;
      if (self.parts[0].index >= form->precomputed_paths.paths.size())
         return 0;
      if (self.parts[1].signature != wrapper_part_types::navi_precomputed_path_node)
         return 0;
      auto& path = form->precomputed_paths.paths[self.parts[0].index];
      lua_pushinteger(L, path.navmeshes.size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (self.parts[0].signature != wrapper_part_types::navi_precomputed_path)
         return 0;
      if (self.parts[0].index >= form->precomputed_paths.paths.size())
         return 0;
      if (self.parts[1].signature != wrapper_part_types::navi_precomputed_path_node)
         return 0;
      auto& path = form->precomputed_paths.paths[self.parts[0].index];
      auto i = lua_tointeger(L, 2) - 1;
      if (i < 0 || i >= path.navmeshes.size())
         return 0;
      return push_native_object(path.navmeshes[i]);
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params navmesh_info_map_precomputed_path_nodes = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}