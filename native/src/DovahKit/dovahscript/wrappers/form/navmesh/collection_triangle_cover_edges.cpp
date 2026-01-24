#include "./collection_triangle_cover_edges.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "../navmesh.h"
#include "./triangle_cover_edge.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.navmesh.triangle_cover_edges>";
}

namespace {
   using namespace dovahscript;

   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }
   
   int get_collection_length(lua_State* L) {
      lua_pushinteger(L, 2);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      if (self.parts[0].signature != wrapper_part_types::navmesh_triangle)
         return 0;
      if (self.parts[1].signature != wrapper_part_types::navmesh_triangle_cover_edge)
         return 0;
      auto i = lua_tointeger(L, 2);
      if (i < 0 || i > 1)
         return 0;
      wrapper out = self;
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::navmesh_triangle_cover_edge::metatable_key);
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params navmesh_triangle_cover_edges = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}