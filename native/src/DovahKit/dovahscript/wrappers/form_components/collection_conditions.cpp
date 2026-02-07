#include "./collection_conditions.h"
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/conditions.h"
#include "dovah/utils/form_component_accessors/condition_list.h"
#include "dovah/forms/FormList.h"
#include "./condition.h"
#include "../form/form.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.condition>";
}

namespace {
   using wrapped_type = dovah::loaded_forms::components::condition_list;
}

namespace dovahscript::wrappers::collections {
   extern dovah::loaded_forms::components::condition_list* unwrap_condition_list(wrapper& self) {
      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      if (!form)
         return nullptr;
      return dovah::utils::form_component_accessors::condition_list(*form);
   }
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
   wrapped_type* get_wrapped_object(wrapper& self) {
      return dovahscript::wrappers::collections::unwrap_condition_list(self);
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* list = get_wrapped_object(self);
      if (!list)
         return 0;
      lua_pushinteger(L, list->size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* list = get_wrapped_object(self);
      if (!list)
         return 0;
      auto  i    = lua_tointeger(L, 2);
      if (i > list->size() || i <= 0)
         return 0;
      --i;
      wrapper out = self;
      assert(out.is_collection);
      assert(out.parts[0].signature == wrapper_part_types::condition_list);
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::condition::metatable_key);
   }
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self     = get_collection_wrapper(L);
      auto* form     = self.get_loaded_form_data<wrapped_type>();
      auto* list_ptr = get_wrapped_object(self);
      if (!list_ptr)
         return 0;
      //
      int  pos_value = 2;
      bool has_index = false;
      //
      if (lua_gettop(L) >= 3) {
         has_index = true;
         pos_value = 3;
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
      }
      dovah::form_stub* target = nullptr;
      if (!lua_isnoneornil(L, pos_value)) {
         auto* w = wrapper_from_stack<wrappers::form>(L, pos_value);
         if (!w)
            cobb::lua::error(L, "you can only insert forms or nil into a formlist");
         target = w->stub;
      }
      //
      if (!form)
         return 0;
      auto& list = *list_ptr;
      auto  size = list.size();
      int   i    = size + 1;
      if (has_index) {
         i = lua_tointeger(L, 2);
         if (i < 1)
            cobb::lua::error(L, "indices below 1, such as %d, are not allowed", i);
         --i;
      }
      self.before_edit();
      if (i >= size) {
         if (i > size) {
            cobb::lua::error(L, "cannot insert past the end of the list");
         }
         list.emplace(list.begin() + i);
      } else {
         list.emplace(list.begin() + i);
      }
      self.after_edit();
      return 0;
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      
      auto& self     = get_collection_wrapper(L);
      auto* form     = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      auto* list_ptr = get_wrapped_object(self);
      luaL_argcheck(L, lua_isinteger(L, 2), 2, "expected an integer index");
      if (!list_ptr)
         return 0;
      auto& list = *list_ptr;
      int   i    = lua_tointeger(L, 2);
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      self.before_edit();
      list[i].clear(*form);
      list.erase(list.begin() + i);
      self.after_edit();
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params condition_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
      //.set_item               = &set_item,
   };
}