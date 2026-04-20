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

   static void table_to_response(lua_State* L, int pos, wrapped_type& form, wrapped_type::response& dst) {
      lua_getfield(L, pos, "edits");
      if (!lua_isnoneornil(L, -1)) {
         dst.edits = lua_tostring(L, -1);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "emotion_type");
      if (!lua_isnoneornil(L, -1)) {
         auto v = lua_tostring(L, -1);
         dst.emotion.type = wrappers::topic_info_response::emotion_from_string(v).value();
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "emotion_value");
      if (!lua_isnoneornil(L, -1)) {
         dst.emotion.value = lua_tointeger(L, -1);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "listener_idle");
      if (!lua_isnoneornil(L, -1)) {
         dovah::form_stub* value = nullptr;
         if (auto* w = wrapper_from_stack<wrappers::form>(L, -1)) {
            value = w->stub;
         }
         dst.idles.listener.set(form, value);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "script_notes");
      if (!lua_isnoneornil(L, -1)) {
         dst.script_notes = lua_tostring(L, -1);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "speaker_idle");
      if (!lua_isnoneornil(L, -1)) {
         dovah::form_stub* value = nullptr;
         if (auto* w = wrapper_from_stack<wrappers::form>(L, -1)) {
            value = w->stub;
         }
         dst.idles.speaker.set(form, value);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "substitute_sound");
      if (!lua_isnoneornil(L, -1)) {
         dovah::form_stub* value = nullptr;
         if (auto* w = wrapper_from_stack<wrappers::form>(L, -1)) {
            value = w->stub;
         }
         dst.sound.set(form, value);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "text");
      if (!lua_isnoneornil(L, -1)) {
         dst.text = lua_tostring(L, -1);
      }
      lua_pop(L, 1);

      lua_getfield(L, pos, "unique_id");
      if (!lua_isinteger(L, -1))
         dst.id = lua_tointeger(L, -1);
      lua_pop(L, 1);
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
            {
               auto message = wrappers::topic_info_response::verify_table_is_response_like(L, pos_value);
               if (!message.empty()) {
                  cobb::lua::error(L, "bad argument #%d to 'topic_info_response_list:insert': %s", pos_value, message.data());
               }
            }
            has_value = true;

            std::optional<uint8_t> unique_id;
            {
               lua_getfield(L, pos_value, "unique_id");
               if (lua_isinteger(L, -1)) {
                  auto v = lua_tointeger(L, -1);
                  assert(v >= 0 && v <= wrapped_type::max_available_response_ids);
                  unique_id = v;
               }
               lua_pop(L, 1);
            }
            if (unique_id.has_value()) {
               has_uid = true;
               for (auto& item : list)
                  if (item.id == unique_id.value())
                     cobb::lua::argerror(L, 2, "the provided table has a `unique_id` which is already in use by an existing response");
            }
         }
      }
      if (!form)
         return 0;

      uint8_t generated_uid = 0;
      if (!has_uid) {
         cobb::bitset<255> used_ids;
         for (auto& item : list) {
            used_ids.set(item.id);
         }
         if (used_ids.all()) {
            cobb::lua::error(L, "this `topic_info` form has no more unique IDs left for new responses");
         }
         used_ids.set(0);
         if (used_ids.all()) {
            cobb::lua::warning(L, "this `topic_info` form only has unique ID 0 available for new responses; this ID is a sentinel value used when recording lines in the Creation Kit");
         } else {
            //
            // TODO: Prefer `highest_set_bit + 1` unless the highest set bit is the 255th bit, 
            //       in which case fall through to the first clear bit.
            //
            generated_uid = used_ids.find_first_clear();
         }
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
            wrappers::topic_info_response::modify_from_table(
               *form,
               list[insert_at],
               L,
               pos_value,
               false
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

      {
         auto message = wrappers::topic_info_response::verify_table_is_response_like(L, index_value);
         if (!message.empty()) {
            cobb::lua::error(L, "bad assignment to 'topic_info_response_list[...]': %s", index_value, message.data());
         }
      }

      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      cobb::lua::argcheck(L, lua_isinteger(L, index_key), index_key, "response indices must be integers");
      auto i = lua_tointeger(L, index_key);
      cobb::lua::argcheck(L, i >= 1, index_key, "indices below 1 are not allowed");
      --i;
      //
      auto& list = form->responses;
      auto  size = list.size();
      if (i >= size) {
         if (i > size) {
            cobb::lua::warning(L, "index %s is out of bounds; empty elements will be created between the end of the list and the new element", lua_tolstring(L, index_key, nullptr));
         }
      }

      {  // enforce that any passed-in unique ID actually is unique within this TopicInfo
         std::optional<uint8_t> unique_id;
         {
            lua_getfield(L, index_value, "unique_id");
            if (lua_isinteger(L, -1)) {
               auto v = lua_tointeger(L, -1);
               assert(v >= 0 && v <= wrapped_type::max_available_response_ids);
               unique_id = v;
            }
            lua_pop(L, 1);
         }
         if (unique_id.has_value()) {
            for (size_t j = 0; j < size; ++j) {
               if (j == i)
                  continue;
               auto& item = list[j];
               if (item.id == unique_id.value())
                  cobb::lua::argerror(L, 2, "the provided table has a `unique_id` which is already in use by a different response");
            }
         }
      }

      self.before_edit();
      if (i >= size)
         list.resize(i + 1);
      wrappers::topic_info_response::modify_from_table(
         *form,
         list[i],
         L,
         index_value,
         true
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