#include "response.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

// For table-to-response ops:
#include "dovahscript/api_helpers/table_contains_expandos.h"
#include "dovahscript/api_helpers/fail_table_if_expandos.h"
#include "dovahscript/api_helpers/subobject_property_helpers/common_lambdas.h"
#include "dovahscript/api_helpers/subobject_property_helpers/make_getters_and_setters.h"
#include "dovahscript/api_helpers/subobject_property_helpers/property_definition.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/localized_string.h"
#include "dovahscript/api_helpers/subobject_property_helpers/types/form_property_definition.h"
//
#include "dovahscript/api_helpers/subobject_property_helpers/apply_table.h"
#include "dovahscript/api_helpers/subobject_property_helpers/verify_table.h"

#define USE_METAPROGRAMMED_ACCESSORS

#include "helpers/attributes/forceinline.h"
#define SUBOBJECT_FIELD_ACCESSOR(prop) [](wrapped_type& src) COBB__FORCEINLINE -> auto& { return src.prop; }

namespace {
   using namespace dovahscript;
   using cls = wrappers::topic_info_response;
   using form_type = dovah::loaded_forms::TopicInfo;
   using wrapped_type = cls::wrapped_type;

   static wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::topic_info_response)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->responses;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }
   static wrapped_type& _unwrap_and_require(lua_State* L) {
      auto& self = get_wrapper_for_thiscall<cls>(L);
      auto* data = _unwrap(self);
      if (data == nullptr)
         cobb::lua::error(L, "topic_info_response wrapper has no underlying object (deleted?)");
      return *data;
   }

   constexpr const auto emotion_type_names = []() {
      using pair_type = std::pair<dovah::dialogue::emotion, std::string_view>;
      return std::array{
         pair_type{ dovah::dialogue::emotion::anger,    "anger" },
         pair_type{ dovah::dialogue::emotion::disgust,  "disgust" },
         pair_type{ dovah::dialogue::emotion::fear,     "fear" },
         pair_type{ dovah::dialogue::emotion::happy,    "happy" },
         pair_type{ dovah::dialogue::emotion::neutral,  "neutral" },
         pair_type{ dovah::dialogue::emotion::puzzled,  "puzzled" },
         pair_type{ dovah::dialogue::emotion::sad,      "sad" },
         pair_type{ dovah::dialogue::emotion::surprise, "surprise" },
      };
   }();
}

/*static*/ std::optional<dovah::dialogue::emotion> cls::emotion_from_string(std::string_view v) {
   for (const auto& pair : emotion_type_names)
      if (v == pair.second)
         return pair.first;
   return {};
}

namespace subobject_property_helpers {
   using namespace api_helpers::subobject_property_helpers;
}

constexpr auto subobject_properties = std::tuple{
   subobject_property_helpers::localized_string_property_definition(
      "edits",
      [](wrapped_type& src) -> auto& { return src.edits; }
   ),

   subobject_property_helpers::property_definition{
      .name   = "emotion_type",
      .access = SUBOBJECT_FIELD_ACCESSOR(emotion.type),
      .check  = [](lua_State* L, int pos) COBB__FORCEINLINE-> std::string_view {
         if (!lua_isstring(L, pos))
            return "string expected";
         auto opt = wrappers::topic_info_response::emotion_from_string(lua_tostring(L, pos));
         if (!opt.has_value())
            return "unrecognized emotion";
         return {};
      },
      .pull = [](lua_State* L, int pos) COBB__FORCEINLINE {
         auto opt = wrappers::topic_info_response::emotion_from_string(lua_tostring(L, pos));
         return opt.value();
      },
      .push = [](lua_State* L, dovah::dialogue::emotion value) COBB__FORCEINLINE {
         for (const auto& pair : emotion_type_names) {
            if (value == pair.first) {
               lua_pushstring(L, pair.second.data());
               return;
            }
         }
         lua_pushnil(L);
      },

      .default_value = dovah::dialogue::emotion::neutral,
   },
   
   subobject_property_helpers::property_definition{
      .name   = "emotion_value",
      .access = SUBOBJECT_FIELD_ACCESSOR(emotion.value),
      .check  = [](lua_State* L, int pos) COBB__FORCEINLINE -> std::string_view {
         if (!lua_isinteger(L, pos))
            return "integer expected";
         auto v = lua_tointeger(L, pos);
         if (v < 0 || v > 100)
            return "value must be in the range [0, 100]";
         return {};
      },
      .pull = &subobject_property_helpers::pull::integer,
      .push = [](lua_State* L, const decltype(decltype(wrapped_type::emotion)::value)& value) COBB__FORCEINLINE { lua_pushinteger(L, value); },

      .default_value = 50,
   },
   
   subobject_property_helpers::define_form_property<dovah::form_type::idle>(
      "listener_idle",
      SUBOBJECT_FIELD_ACCESSOR(idles.listener)
   ),
   
   subobject_property_helpers::localized_string_property_definition(
      "script_notes",
      SUBOBJECT_FIELD_ACCESSOR(script_notes)
   ),

   subobject_property_helpers::define_form_property<dovah::form_type::idle>(
      "speaker_idle",
      SUBOBJECT_FIELD_ACCESSOR(idles.speaker)
   ),

   subobject_property_helpers::define_form_property<dovah::form_type::sound>(
      "substitute_sound",
      SUBOBJECT_FIELD_ACCESSOR(sound)
   ),

   subobject_property_helpers::localized_string_property_definition(
      "text",
      SUBOBJECT_FIELD_ACCESSOR(text)
   ),
   
   subobject_property_helpers::property_definition{
      .name   = "unique_id",
      .access = SUBOBJECT_FIELD_ACCESSOR(id),
      .check  = [](lua_State* L, int pos) -> std::string_view {
         if (!lua_isinteger(L, pos))
            return "integer expected";
         auto v = lua_tointeger(L, pos);
         if (v < 0 || v > form_type::max_available_response_ids)
            return "value is out of bounds";
         return {};
      },
      .pull = &subobject_property_helpers::pull::integer,
      .push = [](lua_State* L, const decltype(wrapped_type::id)& value) { lua_pushinteger(L, value); },

      .default_value = 50,
   },
};

/*static*/ std::string cls::verify_table_is_response_like(lua_State* L, int pos) {
   return subobject_property_helpers::verify_table<wrapped_type, true, subobject_properties>(L, pos);
}

/*static*/ void cls::modify_from_table(
   dovah::loaded_forms::Form& dst_form,
   wrapped_type& dst_data,
   lua_State* L,
   int  table_pos,
   bool is_overwrite
) {
   if (is_overwrite) {
      subobject_property_helpers::apply_table<wrapped_type, true, subobject_properties>(L, table_pos, dst_form, dst_data);
   } else {
      subobject_property_helpers::apply_table<wrapped_type, false, subobject_properties>(L, table_pos, dst_form, dst_data);
   }
}

namespace {
   namespace _methods {
      int assign(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         {
            auto message = cls::verify_table_is_response_like(L, 2);
            if (!message.empty())
               cobb::lua::error(L, "bad argument #%d to 'topic_info_response:assign': %s", 2, message.data());
         }
         if (!data)
            return 0;
         self.before_edit();
         cls::modify_from_table(
            *self.form,
            *data,
            L,
            2,
            false
         );
         self.after_edit();
         return 0;
      }
      int overwrite_with(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         {
            auto message = cls::verify_table_is_response_like(L, 2);
            if (!message.empty())
               cobb::lua::error(L, "bad argument #%d to 'topic_info_response:overwrite_with': %s", 2, message.data());
         }
         if (!data)
            return 0;
         self.before_edit();
         cls::modify_from_table(
            *self.form,
            *data,
            L,
            2,
            true
         );
         self.after_edit();
         return 0;
      }
   }
   namespace _getters {
      int edits(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         lua_pushstring(L, data.edits.c_str());
         return 1;
      }
      int emotion_type(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         for (const auto& pair : emotion_type_names) {
            if (data.emotion.type == pair.first) {
               lua_pushstring(L, pair.second.data());
               return 1;
            }
         }
         return 0;
      }
      int emotion_value(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         lua_pushinteger(L, data.emotion.value);
         return 1;
      }
      int listener_idle(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         return push_native_object(data.idles.listener);
      }
      int parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         return push_native_object(self.stub);
      }
      int script_notes(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         lua_pushstring(L, data.script_notes.c_str());
         return 1;
      }
      int speaker_idle(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         return push_native_object(data.idles.speaker);
      }
      int substitute_sound(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         return push_native_object(data.sound);
      }
      int text(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         lua_pushstring(L, data.text.c_str());
         return 1;
      }
      int unique_id(lua_State* L) {
         auto& data = _unwrap_and_require(L);
         lua_pushinteger(L, data.id);
         return 1;
      }
   }
   namespace _setters {
      int edits(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = _unwrap(self);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!data)
            return 0;
         self.before_edit();
         data->edits = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int emotion_type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_and_require(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         dovah::dialogue::emotion v;
         {
            std::string_view raw = lua_tostring(L, 2);
            auto opt = wrappers::topic_info_response::emotion_from_string(raw);
            if (!opt.has_value())
               cobb::lua::argerror(L, 2, "unrecognized emotion type");
            v = opt.value();
         }
         self.before_edit();
         data.emotion.type = v;
         self.after_edit();
         return 1;
      }
      int emotion_value(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_and_require(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "integer expected");
         auto v = lua_tointeger(L, 2);
         if (v < 0 || v > 100)
            luaL_argerror(L, 2, "emotion values must be in the range [0, 100]");

         self.before_edit();
         data.emotion.value = v;
         self.after_edit();
         return 1;
      }
      int listener_idle(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* word  = _unwrap(self);
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::idle);
         if (!word)
            return 0;
         self.before_edit();
         word->idles.listener.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
      int script_notes(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = _unwrap(self);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!data)
            return 0;
         self.before_edit();
         data->script_notes = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int speaker_idle(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* word  = _unwrap(self);
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::idle);
         if (!word)
            return 0;
         self.before_edit();
         word->idles.speaker.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
      int substitute_sound(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* word  = _unwrap(self);
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::sound);
         if (!word)
            return 0;
         self.before_edit();
         word->sound.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
      int text(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = _unwrap(self);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!data)
            return 0;
         self.before_edit();
         data->text = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int unique_id(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_and_require(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "integer expected");
         auto v = lua_tointeger(L, 2);
         if (v < 0 || v > form_type::max_available_response_ids)
            luaL_argerror(L, 2, "response IDs must be in the range [0, 255]");
         if (v == 0)
            lua_warning(L, "a response ID of 0 is invalid (it is the sentinel used while recording lines in the Creation Kit)", 0);

         if (data.id == v)
            return 0;
         auto* form = self.get_loaded_form_data<form_type>();
         for (auto& resp : form->responses) {
            if (resp.id == v && &resp != &data) {
               luaL_argerror(L, 2, "the provided response ID is already in use by another response on this TopicInfo");
            }
         }

         self.before_edit();
         data.id = v;
         self.after_edit();
         return 1;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "assign",         &_methods::assign },
      { "overwrite_with", &_methods::overwrite_with },
   };
   
   #ifdef USE_METAPROGRAMMED_ACCESSORS
      /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
         { "parent", &_getters::parent }, // get containing info
      };
      /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;

      /*static*/ void cls::extra_class_setup(lua_State* L) {
         subobject_property_helpers::setup_extra_getters_and_setters<cls, &_unwrap_and_require, subobject_properties>(L);
      }
   #else
      /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
         { "edits",            &_getters::edits },
         { "emotion_type",     &_getters::emotion_type },
         { "emotion_value",    &_getters::emotion_value },
         { "listener_idle",    &_getters::listener_idle },
         { "parent",           &_getters::parent }, // get containing info
         { "script_notes",     &_getters::script_notes },
         { "speaker_idle",     &_getters::speaker_idle },
         { "substitute_sound", &_getters::substitute_sound },
         { "text",             &_getters::text },
         { "unique_id",        &_getters::unique_id },
      };
      /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
         { "edits",            &_setters::edits },
         { "emotion_type",     &_setters::emotion_type },
         { "emotion_value",    &_setters::emotion_value },
         { "listener_idle",    &_setters::listener_idle },
         { "script_notes",     &_setters::script_notes },
         { "speaker_idle",     &_setters::speaker_idle },
         { "substitute_sound", &_setters::substitute_sound },
         { "text",             &_setters::text },
         { "unique_id",        &_setters::unique_id },
      };

      /*static*/ void cls::extra_class_setup(lua_State* L) {
      }
   #endif
}