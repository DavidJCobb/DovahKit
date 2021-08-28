#include "word.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Shout.h"
#include "../shout.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::shout_word;
   using form_type    = dovah::loaded_forms::Shout;
   using wrapped_type = cls::wrapped_type;

   wrapped_type* _unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[0].signature != wrapper_part_types::shout_word)
         return nullptr;
      auto* form = w.get_loaded_form_data<form_type>();
      if (!form)
         return nullptr;
      auto& list = form->words;
      auto  i = w.parts[0].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }

   namespace _getters {
      int word_of_power(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* word = _unwrap(self);
         if (word == nullptr)
            cobb::lua::error(L, "shout_word wrapper has no underlying object (deleted?)");
         return push_native_object(word->word_of_power);
      }
      int spell(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* word = _unwrap(self);
         if (word == nullptr)
            cobb::lua::error(L, "shout_word wrapper has no underlying object (deleted?)");
         return push_native_object(word->spell);
      }
      int cooldown(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* word = _unwrap(self);
         if (word == nullptr)
            cobb::lua::error(L, "shout_word wrapper has no underlying object (deleted?)");
         lua_pushnumber(L, word->recoveryTime);
         return 1;
      }
   }
   namespace _setters {
      int word_of_power(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* word  = _unwrap(self);
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::word_of_power);
         if (!word)
            return 0;
         self.before_edit();
         word->word_of_power.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
      int spell(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* word  = _unwrap(self);
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::spell);
         if (!word)
            return 0;
         self.before_edit();
         word->spell.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
      int cooldown(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* word = _unwrap(self);
         if (!word)
            return 0;
         self.before_edit();
         word->recoveryTime = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "word_of_power", &_getters::word_of_power },
      { "spell",         &_getters::spell },
      { "cooldown",      &_getters::cooldown },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "word_of_power", &_setters::word_of_power },
      { "spell",         &_setters::spell },
      { "cooldown",      &_setters::cooldown },
   };
}