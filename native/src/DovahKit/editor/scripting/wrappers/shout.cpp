#include "shout.h"
#include "../classes.h"
#include "../util.h"
#include "../editor_script_core.h"
#include "../wrapper_util.h"
#include "../collections.h"

#include "shout/word.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::shout;
   using _loaded_form_t = dovah::loaded_forms::Shout;
}

#pragma region Collection: "words"
namespace {
   using namespace editor_script;

   namespace _collections::words {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, _wrapper_t::word_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", _wrapper_t::word_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         lua_pushnumber(L, 3);
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto  i    = lua_tointeger(L, 2);
         auto& list = form->words;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         wrapper out = self;
         assert(out.is_collection);
         assert(out.parts[0].signature == editor_script::wrapper_part_types::shout_word);
         out.into_collection(i);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::shout_word::metatable_key);
      }
   }
}
#pragma endregion

#pragma region shout
namespace {
   namespace _getters {
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      luastackchange_t description(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->description.c_str());
         return 1;
      }
      luastackchange_t equip_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out;
         auto* mt = wrap_form(out, form->equip_type.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t menu_display_object(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out;
         auto* mt = wrap_form(out, form->menu_display_object.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t words(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(editor_script::wrapper_part_types::shout_word);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, _wrapper_t::word_collection_key);
      }
   }
   namespace _setters {
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t description(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->description = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t equip_type(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::equip_slot, true);
         if (!form)
            return 0;
         self.before_edit();
         form->equip_type.set(*self.stub, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t menu_display_object(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::statik, true);
         if (!form)
            return 0;
         self.before_edit();
         form->menu_display_object.set(*self.stub, other->stub);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "name",        &_getters::name },
      { "description", &_getters::description },
      { "equip_type",  &_getters::equip_type },
      { "menu_display_object", &_getters::menu_display_object },
      { "words",       &_getters::words },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "name",        &_setters::name },
      { "description", &_setters::description },
      { "equip_type",  &_setters::equip_type },
      { "menu_display_object", &_setters::menu_display_object },
   };

   /*static*/ void _wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(
         L,
         _wrapper_t::word_collection_key,
         &wrapper::__gc,
         false,
         &_collections::words::get_collection_length, // args: wrapper;        return: number
         nullptr,
         &_collections::words::lookup_item_by_index,  // args: wrapper, index; return: wrapper or nil
         nullptr
      );
   }
}
#pragma endregion