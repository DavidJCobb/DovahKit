#include "word.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::shout_word;
   using _loaded_form_t = dovah::loaded_forms::Shout;
}

#pragma region shout_word
namespace {
   namespace _getters {
      luastackchange_t word_of_power(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::shout_word>(L);
         auto* word = wrappers::shout_word::unwrap(self);
         if (word == nullptr)
            luaL_error(L, "shout_word wrapper has no underlying object (deleted?)");
         __assume(word != nullptr);
         wrapper out;
         auto* mt = wrap_form(out, word->word_of_power.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t spell(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::shout_word>(L);
         auto* word = wrappers::shout_word::unwrap(self);
         if (word == nullptr)
            luaL_error(L, "shout_word wrapper has no underlying object (deleted?)");
         __assume(word != nullptr);
         wrapper out;
         auto* mt = wrap_form(out, word->spell.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t cooldown(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::shout_word>(L);
         auto* word = wrappers::shout_word::unwrap(self);
         if (word == nullptr)
            luaL_error(L, "shout_word wrapper has no underlying object (deleted?)");
         __assume(word != nullptr);
         lua_pushnumber(L, word->recoveryTime);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t word_of_power(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* word  = wrappers::shout_word::unwrap(self);
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::word_of_power, true);
         if (!word)
            return 0;
         self.before_edit();
         word->word_of_power.set(*self.stub, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t spell(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* word  = wrappers::shout_word::unwrap(self);
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::spell, true);
         if (!word)
            return 0;
         self.before_edit();
         word->spell.set(*self.stub, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t cooldown(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* word = wrappers::shout_word::unwrap(self);
         if (!word)
            return 0;
         self.before_edit();
         word->recoveryTime = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "word_of_power", &_getters::word_of_power },
      { "spell",         &_getters::spell },
      { "cooldown",      &_getters::cooldown },
   };
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "word_of_power", &_setters::word_of_power },
      { "spell",         &_setters::spell },
      { "cooldown",      &_setters::cooldown },
   };

   /*static*/ _wrapper_t::wrapped_t* _wrapper_t::unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != editor_script::wrapper_part_types::shout_word)
         return nullptr;
      auto* form = w.get_loaded_form_data<_loaded_form_t>();
      if (!form)
         return nullptr;
      auto& list = form->words;
      auto  i    = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }
}
#pragma endregion