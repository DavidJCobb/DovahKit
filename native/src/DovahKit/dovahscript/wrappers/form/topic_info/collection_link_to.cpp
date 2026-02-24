#include "./collection_link_to.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/TopicInfo.h"
#include "../topic.h"
#include "../topic_info.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.topic_info.link_to>";
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

   template<bool AddingAnElement>
   size_t _pull_and_check_index(lua_State* L, wrapped_type& form, int arg) {
      cobb::lua::argcheck(L, lua_isinteger(L, arg), arg, "provided index is not an integer");
      auto insert_at = lua_tointeger(L, arg);
      if (insert_at < 1)
         cobb::lua::error(L, "indices below 1, such as %d, are not allowed", insert_at);
      size_t locked_count = form.link_to.locked.size();
      size_t total_count  = locked_count + form.link_to.normal.size();
      if constexpr (AddingAnElement) {
         if (insert_at > total_count)
            cobb::lua::error(L, "index %d is out of bounds", insert_at);
      } else {
         if (insert_at > total_count)
            cobb::lua::error(L, "index %d is out of bounds", insert_at);
      }
      if (insert_at <= locked_count)
         cobb::lua::error(L, "cannot modify the locked half of the list");
      return insert_at - locked_count - 1;
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form) {
         lua_pushinteger(L, 0);
         return 1;
      }
      lua_pushinteger(L, form->link_to.locked.size() + form->link_to.normal.size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto i = lua_tointeger(L, 2);
      if (i <= 0)
         return 0;
      --i;

      auto locked_size = form->link_to.locked.size();
      if (i < locked_size) {
         return push_native_object(form->link_to.locked[i]);
      } else {
         i -= locked_size;
         if (i < form->link_to.normal.size()) {
            return push_native_object(form->link_to.normal[i]);
         }
      }
      return 0;
   }
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self  = get_collection_wrapper(L);
      auto* form  = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;

      size_t max_index = form->link_to.normal.size() - 1;
      int    insert_at = form->link_to.locked.size() + form->link_to.normal.size();
      dovah::form_stub* item = nullptr;
      {
         int pos_value = 2;
         if (lua_gettop(L) >= 3) {
            pos_value = 3;
            insert_at = _pull_and_check_index<true>(L, *form, 2);
         }

         if (!lua_isnoneornil(L, pos_value)) {
            auto* w = wrapper_from_stack<wrappers::topic>(L, pos_value);
            if (!w)
               cobb::lua::error(L, "you can only insert topics");
            item = w->stub;
         }
         cobb::lua::argcheck(L, !!item, pos_value, "cannot insert a nil topic");
      }
      if (!form)
         return 0;

      self.before_edit();
      {
         auto& list = form->link_to.normal;
         list.emplace(list.begin() + insert_at);
         list[insert_at].set(*form, item);
      }
      self.after_edit();
      return 0;
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto& list = form->link_to.normal;

      size_t remove_at = _pull_and_check_index<false>(L, *form, 2);
      self.before_edit();
      list[remove_at].set(*form, nullptr);
      list.erase(list.begin() + remove_at);
      self.after_edit();
      return 0;
   }
   int set_item(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      
      constexpr auto index_self  = 1;
      constexpr auto index_key   = 2;
      constexpr auto index_value = 3;

      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto& list = form->link_to.normal;

      size_t assign_at = _pull_and_check_index<false>(L, *form, index_key);

      dovah::form_stub* target = nullptr;
      if (!lua_isnoneornil(L, index_value)) {
         auto* w = wrapper_from_stack<wrappers::topic>(L, index_value);
         if (!w)
            cobb::lua::error(L, "you can only assign topics to this list");
         target = w->stub;
      }

      self.before_edit();
      list[assign_at].set(*form, target);
      self.after_edit();
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params topic_info_link_to = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
      .set_item               = &set_item,
   };
}