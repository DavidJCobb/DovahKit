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

   static void verify_table_arg_is_response_like(lua_State* L, int pos) {
      switch (lua_type(L, pos)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            cobb::lua::argerror(L, pos, "expected table or userdata");
      }
      api_helpers::fail_table_if_expandos(L, pos, std::array{
         std::string_view("edits"),
         std::string_view("listener_idle"),
         std::string_view("script_notes"),
         std::string_view("speaker_idle"),
         std::string_view("substitute_sound"),
         std::string_view("text"),
      });

      lua_getfield(L, pos, "edits");
      cobb::lua::argcheck(L, lua_isnoneornil(L, -1) || lua_isstring(L, -1), pos, "table's `edits` field is not a string");
      lua_pop(L, 1);
      {
         lua_getfield(L, pos, "listener_idle");
         if (!lua_isnoneornil(L, -1)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, -1);
            if (!w || w->stub->form_type != dovah::form_type::idle)
               cobb::lua::argerror(L, pos, "table's `listener_idle` field is not an idle animation");
         }
         lua_pop(L, 1);
      }
      lua_getfield(L, pos, "script_notes");
      cobb::lua::argcheck(L, lua_isnoneornil(L, -1) || lua_isstring(L, -1), pos, "table's `script_notes` field is not a string");
      lua_pop(L, 1);
      {
         lua_getfield(L, pos, "speaker_idle");
         if (!lua_isnoneornil(L, -1)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, -1);
            if (!w || w->stub->form_type != dovah::form_type::idle)
               cobb::lua::argerror(L, pos, "table's `listener_idle` field is not an idle form");
         }
         lua_pop(L, 1);
      }
      {
         lua_getfield(L, pos, "substitute_sound");
         if (!lua_isnoneornil(L, -1)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, -1);
            if (!w || w->stub->form_type != dovah::form_type::sound)
               cobb::lua::argerror(L, pos, "table's `substitute_sound` field is not a sound form");
         }
         lua_pop(L, 1);
      }
      lua_getfield(L, pos, "text");
      cobb::lua::argcheck(L, lua_isnoneornil(L, -1) || lua_isstring(L, -1), pos, "table's `text` field is not a string");
      lua_pop(L, 1);
   }
   static void table_to_response(lua_State* L, int pos, wrapped_type& form, wrapped_type::response& dst) {
      lua_getfield(L, pos, "edits");
      if (!lua_isnoneornil(L, -1)) {
         dst.edits = lua_tostring(L, -1);
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
      {
         if (lua_gettop(L) >= 3) {
            pos_value = 3;

            cobb::lua::argcheck(L, lua_isinteger(L, 2), 2, "provided index must be an integer");
            auto i = lua_tointeger(L, 2);
            cobb::lua::argcheck(L, i >= 1, 2, "cannot insert at a zero or negative index");
            insert_at = i - 1;
         }

         if (!lua_isnoneornil(L, pos_value)) {
            verify_table_arg_is_response_like(L, pos_value);
            has_value = true;
         }
      }
      if (!form)
         return 0;

      self.before_edit();
      {
         if (insert_at >= list.size()) {
            list.resize(insert_at + 1);
         } else {
            list.emplace(list.begin() + insert_at);
            core::subsystems::userdata::get().insert_into_sequential_collection(self, insert_at);
         }
         if (has_value) {
            table_to_response(L, pos_value, *form, list[insert_at]);
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

      verify_table_arg_is_response_like(L, index_value);

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
         list.resize(i + 1);
      }
      self.before_edit();
      table_to_response(L, index_value, *form, list[i]);
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