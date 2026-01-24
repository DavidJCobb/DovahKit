#include "./collection_vertices.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Navmesh.h"
#include "../navmesh.h"
#include "./vertex.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.navmesh.vertices>";
}

namespace {
   using namespace dovahscript;

   using wrapped_type = dovah::loaded_forms::Navmesh;
   
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
      if (!form)
         return 0;
      auto& list = form->geometry.vertices;
      lua_pushinteger(L, list.size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto  i    = lua_tointeger(L, 2);
      auto& list = form->geometry.vertices;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      wrapper out = self;
      assert(out.is_collection);
      assert(out.parts[0].signature == wrapper_part_types::navmesh_vertex);
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::navmesh_vertex::metatable_key);
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params navmesh_vertices = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}