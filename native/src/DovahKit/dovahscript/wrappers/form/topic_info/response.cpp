#include "response.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/TopicInfo.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::topic_info_response;
   using form_type    = dovah::loaded_forms::TopicInfo;
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
         pair_type{ dovah::dialogue::emotion::anger, "anger" },
         pair_type{ dovah::dialogue::emotion::disgust, "disgust" },
         pair_type{ dovah::dialogue::emotion::fear, "fear" },
         pair_type{ dovah::dialogue::emotion::happy, "happy" },
         pair_type{ dovah::dialogue::emotion::neutral, "neutral" },
         pair_type{ dovah::dialogue::emotion::puzzled, "puzzled" },
         pair_type{ dovah::dialogue::emotion::sad, "sad" },
         pair_type{ dovah::dialogue::emotion::surprise, "surprise" },
      };
   }();

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
            bool found = false;
            for (const auto& pair : emotion_type_names) {
               if (raw == pair.second) {
                  v     = pair.first;
                  found = true;
                  break;
               }
            }
            if (!found)
               cobb::lua::argerror(L, 2, "unrecognized emotion type");
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
   }
}

namespace dovahscript::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   
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
   };
}