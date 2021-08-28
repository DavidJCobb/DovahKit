#include "shout.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/Shout.h"
#include "shout/word.h"
#include "shout/collection_words.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::shout;
   using wrapped_type = cls::wrapped_type;

   namespace _getters {
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      int description(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->description.c_str());
         return 1;
      }
      int equip_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->equip_type);
      }
      int menu_display_object(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->menu_display_object);
      }
      int words(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::shout_word);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::shout_words.registry_key);
      }
   }
   namespace _setters {
      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int description(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->description = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int equip_type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::equip_slot);
         if (!form)
            return 0;
         self.before_edit();
         form->equip_type.set(*form, value);
         self.after_edit();
         return 0;
      }
      int menu_display_object(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::statik);
         if (!form)
            return 0;
         self.before_edit();
         form->menu_display_object.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "name",        &_getters::name },
      { "description", &_getters::description },
      { "equip_type",  &_getters::equip_type },
      { "menu_display_object", &_getters::menu_display_object },
      { "words",       &_getters::words },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "name",        &_setters::name },
      { "description", &_setters::description },
      { "equip_type",  &_setters::equip_type },
      { "menu_display_object", &_setters::menu_display_object },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, collections::shout_words);
   }
}
#pragma endregion