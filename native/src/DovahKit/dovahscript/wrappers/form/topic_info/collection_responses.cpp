#include "collection_responses.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/TopicInfo.h"
#include "../topic_info.h"
#include "response.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.topic_info.responses>";
}

namespace {
   using namespace dovahscript;

   using wrapped_type = dovah::loaded_forms::TopicInfo;
   
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
      if (!form) {
         lua_pushinteger(L, 0);
         return 1;
      }
      lua_pushinteger(L, form->responses.size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto  i    = lua_tointeger(L, 2);
      auto& list = form->responses;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      wrapper out = self;
      assert(out.is_collection);
      assert(out.parts[0].signature == wrapper_part_types::topic_info_response);
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::topic_info_response::metatable_key);
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params topic_info_responses = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}