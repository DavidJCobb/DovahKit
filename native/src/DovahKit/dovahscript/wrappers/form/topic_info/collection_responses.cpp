#include "./collection_responses.h"
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "dovahscript/api_helpers/fail_table_if_expandos.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/TopicInfo.h"
#include "../topic_info.h"
#include "./response.h"

#include "dovahscript/api_helpers/subobject_property_helpers/verify_table_for_lua_assignment.h"

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

   static std::optional<uint8_t> generate_unique_id(auto& list) {
      cobb::bitset<256> used_ids;
      for (auto& item : list)
         used_ids.set(item.id);

      if (used_ids.all())
         return {};
      used_ids.set(0);
      if (used_ids.all())
         return 0;
      //
      // TODO: Prefer `highest_set_bit + 1` unless the highest set bit is the 255th bit, 
      //       in which case fall through to the first clear bit.
      //
      return used_ids.find_first_clear();
   }

   static std::vector<uint8_t> generate_unique_ids(auto& list, size_t count) {
      cobb::bitset<256> used_ids;
      for (auto& item : list)
         used_ids.set(item.id);
      if (used_ids.all())
         return {};

      bool zero_is_taken = used_ids.test(0);
      used_ids.set(0);

      std::vector<uint8_t> result;
      for (size_t i = 0; i < count; ++i) {
         auto id = used_ids.find_first_clear();
         if (id >= 0) {
            result.push_back(id);
            continue;
         }
         //
         // All good IDs are taken. Abort.
         //
         if (!zero_is_taken) {
            //
            // ...though if zero isn't taken, use that and then abort.
            //
            result.push_back(0);
         }
         break;
      }
      return result;
   }

   // ---
   
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
   int member_function_insert(lua_State* L) {
      using subobject_wrapper = wrappers::topic_info_response;
      using subobject_type    = subobject_wrapper::wrapped_type;

      core::subsystems::permissions::verify_form_write_permissions();
      
      auto& self  = get_collection_wrapper(L);
      auto* form  = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto& list = form->responses;

      int  insert_at = form->responses.size();
      int  pos_value = 2;
      bool has_value = false;
      bool has_uid   = false;
      {
         if (lua_gettop(L) >= 3) {
            pos_value = 3;

            cobb::lua::argcheck(L, lua_isinteger(L, 2), 2, "provided index must be an integer");
            auto i = lua_tointeger(L, 2);
            cobb::lua::argcheck(L, i >= 1, 2, "cannot insert at a zero or negative index");
            insert_at = i - 1;
         }
         if (!lua_isnoneornil(L, pos_value)) {
            wrappers::topic_info_response::verify_table_for_insertion(L, pos_value, *form);
            has_value = true;
         }
      }
      if (!form)
         return 0;

      uint8_t generated_uid = 0;
      if (!has_uid) {
         auto opt = generate_unique_id(list);
         if (!opt.has_value())
            cobb::lua::error(L, "this `topic_info` form has no more unique IDs left for new responses");
         generated_uid = opt.value();
         if (generated_uid == 0)
            cobb::lua::warning(L, "this `topic_info` form only has unique ID 0 available for new responses; this ID is a sentinel value used when recording lines in the Creation Kit");
      }

      self.before_edit();
      {
         if (insert_at >= list.size()) {
            list.resize(insert_at + 1);
         } else {
            list.emplace(list.begin() + insert_at);
            core::subsystems::userdata::get().insert_into_sequential_collection(self, insert_at);
         }
         if (has_value) {
            wrappers::topic_info_response::overwrite_with_table(
               *form,
               list[insert_at],
               L,
               pos_value
            );
         }
         if (!has_uid) {
            list[insert_at].id = generated_uid;
         }
      }
      self.after_edit();
      return 0;
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      luaL_argcheck(L, lua_isinteger(L, 2), 2, "expected an integer index");
      int i = lua_tointeger(L, 2);
      if (!form)
         return 0;
      auto& list = form->responses;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      self.before_edit();
      {
         list[i].clear(*form);
         list.erase(list.begin() + i);
      }
      self.after_edit();
      {
         wrapper to_remove = self;
         to_remove.into_collection(i);
         core::subsystems::userdata::get().remove_from_sequential_collection(to_remove);
      }
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
      cobb::lua::argcheck(L, lua_isinteger(L, index_key), index_key, "response indices must be integers");
      auto i = lua_tointeger(L, index_key);
      cobb::lua::argcheck(L, i >= 1, index_key, "indices below 1 are not allowed");
      --i;
      
      std::vector<uint8_t> uids_to_generate;

      auto& list = form->responses;
      auto  size = list.size();
      if (i > size) {
         cobb::lua::warning(L, "index %s is out of bounds; empty elements will be created between the end of the list and the new element", lua_tolstring(L, index_key, nullptr));
         wrappers::topic_info_response::verify_table_for_insertion(L, index_value, *form);

         {
            size_t count_to_create = i - size + 1;
            size_t count_to_auto   = count_to_create;
            {
               lua_getfield(L, index_value, "unique_id");
               if (!lua_isnoneornil(L, -1))
                  --count_to_auto;
               lua_pop(L, 1);
            }

            uids_to_generate = generate_unique_ids(list, count_to_auto);
            if (uids_to_generate.empty()) {
               cobb::lua::error(L, "this `topic_info` form has no more unique IDs left for new responses");
            }
            if (uids_to_generate.size() < count_to_auto) {
               cobb::lua::error(L, "this `topic_info` form doesn't have enough unique IDs left for the %u new responses you are attempting to create", (int)count_to_create);
            }
            if (uids_to_generate.back() == 0) {
               cobb::lua::warning(L, "one of the new responses will end up using unique ID 0; this ID is a sentinel value used when recording lines in the Creation Kit");
            }
         }
      } else {
         wrappers::topic_info_response::verify_table_can_overwrite(L, index_value, *form, list[i]);

         bool has_uid = false;
         {
            lua_getfield(L, index_value, "unique_id");
            if (!lua_isnoneornil(L, -1))
               has_uid = true;
            lua_pop(L, 1);
         }
         if (!has_uid) {
            auto opt = generate_unique_id(list);
            if (!opt.has_value())
               cobb::lua::error(L, "this `topic_info` form has no more unique IDs left for new responses");
            auto val = opt.value();
            if (val == 0)
               cobb::lua::warning(L, "this `topic_info` form only has unique ID 0 available for new responses; this ID is a sentinel value used when recording lines in the Creation Kit");
            uids_to_generate.push_back(val);
         }
      }

      self.before_edit();
      if (i >= size)
         list.resize(i + 1);
      for (size_t j = 0; j < uids_to_generate.size(); ++j) {
         list[size + j].id = uids_to_generate[j];
      }
      wrappers::topic_info_response::overwrite_with_table(
         *form,
         list[i],
         L,
         index_value
      );
      self.after_edit();
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params topic_info_responses = {
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