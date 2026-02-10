#include "./collection_fragments_indexed.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/papyrus/attachment_data.h"
#include "dovah/forms/components/papyrus/fragment_data/perk_fragment_data.h"
#include "../papyrus.h"
#include "./fragment.h"
#include "dovah/forms/Form.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.papyrus.fragment>";
}

namespace {
   using namespace dovahscript;
   using root_wrapper = wrappers::papyrus_root;

   using fragment_type = dovah::loaded_forms::components::papyrus::fragment_type;
   //
   using perk_fragment_data = dovah::loaded_forms::components::papyrus::perk_fragment_data;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }
   perk_fragment_data* unwrap(wrapper& w, root_wrapper::wrapped_type& root) {
      auto* base = root.fragment_data;
      if (!base)
         return nullptr;
      if (base->type != fragment_type::perk)
         return nullptr;
      return (perk_fragment_data*)base;
   }

   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;
      auto* frag = unwrap(self, *root);
      if (!frag) {
         lua_pushinteger(L, 0);
         return 1;
      }
      lua_pushinteger(L, frag->fragments.size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;
      auto* frag = unwrap(self, *root);
      if (!frag)
         return 0;

      auto  i    = lua_tointeger(L, 2);
      auto& list = frag->fragments;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      wrapper out = self;
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_fragment::metatable_key);
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params papyrus_fragments_indexed = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}