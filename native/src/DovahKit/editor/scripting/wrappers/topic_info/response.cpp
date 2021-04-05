#include "response.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::topic_info_response;
   using _loaded_form_t = dovah::loaded_forms::TopicInfo;
}

#pragma region shout_word
namespace {
   namespace _getters {
      luastackchange_t edits(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         lua_pushstring(L, data->edits.c_str());
         return 1;
      }
      luastackchange_t listener_idle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         wrapper out;
         auto* mt = wrap_form(out, data->idles.listener.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         wrapper out;
         auto* mt = wrap_form(out, self.stub);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t script_notes(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         lua_pushstring(L, data->script_notes.c_str());
         return 1;
      }
      luastackchange_t speaker_idle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         wrapper out;
         auto* mt = wrap_form(out, data->idles.speaker.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t substitute_sound(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         wrapper out;
         auto* mt = wrap_form(out, data->sound.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data = _wrapper_t::unwrap(self);
         if (data == nullptr)
            luaL_error(L, "topic_info_response wrapper has no underlying object (deleted?)");
         __assume(data != nullptr);
         lua_pushstring(L, data->text.c_str());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t edits(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data  = _wrapper_t::unwrap(self);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!data)
            return 0;
         self.before_edit();
         data->edits = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t listener_idle(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* word  = _wrapper_t::unwrap(self);
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::idle, true);
         if (!word)
            return 0;
         self.before_edit();
         word->idles.listener.set(*self.stub->form, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t script_notes(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data  = _wrapper_t::unwrap(self);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!data)
            return 0;
         self.before_edit();
         data->script_notes = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t speaker_idle(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* word  = _wrapper_t::unwrap(self);
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::idle, true);
         if (!word)
            return 0;
         self.before_edit();
         word->idles.speaker.set(*self.stub->form, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t substitute_sound(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* word  = _wrapper_t::unwrap(self);
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::idle, true);
         if (!word)
            return 0;
         self.before_edit();
         word->sound.set(*self.stub->form, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* data  = _wrapper_t::unwrap(self);
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
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "edits",            &_getters::edits },
      { "listener_idle",    &_getters::listener_idle },
      { "parent",           &_getters::parent }, // get containing info
      { "script_notes",     &_getters::script_notes },
      { "speaker_idle",     &_getters::speaker_idle },
      { "substitute_sound", &_getters::substitute_sound },
      { "text",             &_getters::text },
   };
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "edits",            &_setters::edits },
      { "listener_idle",    &_setters::listener_idle },
      { "script_notes",     &_setters::script_notes },
      { "speaker_idle",     &_setters::speaker_idle },
      { "substitute_sound", &_setters::substitute_sound },
      { "text",             &_setters::text },
   };

   /*static*/ _wrapper_t::wrapped_t* _wrapper_t::unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != editor_script::wrapper_part_types::topic_info_response)
         return nullptr;
      auto* form = w.get_loaded_form_data<_loaded_form_t>();
      if (!form)
         return nullptr;
      auto& list = form->responses;
      auto  i    = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }
}
#pragma endregion