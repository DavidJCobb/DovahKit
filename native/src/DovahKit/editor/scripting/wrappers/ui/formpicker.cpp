#include "formpicker.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "../../api/form_type_values.h"
#include "../form.h"

namespace {
   // Default form types to limit new FormPicker widgets to. These are the types 
   // allowed by the normal GetIsID condition function.
   constexpr std::initializer_list<uint8_t> default_form_type_filter = {
      dovah::form_type::acoustic_space,
      dovah::form_type::activator,
      dovah::form_type::actor_base,
      dovah::form_type::container,
      dovah::form_type::door,
      dovah::form_type::flora,
      dovah::form_type::furniture,
      dovah::form_type::grass,
      dovah::form_type::hazard,
      dovah::form_type::idle_marker,
      dovah::form_type::light,
      dovah::form_type::movable_static,
      dovah::form_type::projectile,
      dovah::form_type::sound,
      dovah::form_type::statik,
      dovah::form_type::talking_activator,
      dovah::form_type::tree,
      // Items:
      dovah::form_type::ammo,
      dovah::form_type::armor,
      dovah::form_type::armor_addon,
      dovah::form_type::book,
      dovah::form_type::key,
      dovah::form_type::leveled_item,
      dovah::form_type::misc_item,
      dovah::form_type::potion,
      dovah::form_type::scroll,
      dovah::form_type::soul_gem,
      dovah::form_type::weapon,
      // Magic:
      dovah::form_type::enchantment,
      dovah::form_type::leveled_spell,
      dovah::form_type::shout,
      dovah::form_type::spell,
      // Other:
      dovah::form_type::formlist,
   };
}

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::formpicker;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget]() { widget->clear(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t allow_none(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->allowNone(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t default_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         dovah::form_stub* result = nullptr;
         {
            auto* widget  = (wrapped_type*)self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->defaultForm(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         wrapper out;
         auto* mt = wrap_form(out, result);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         dovah::form_stub* result = nullptr;
         {
            auto* widget  = (wrapped_type*)self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->formStub(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         wrapper out;
         auto* mt = wrap_form(out, result);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t form_types(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QVector<dovah::form_type_t> result;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->allowedFormTypes(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         int size = result.size();
         lua_createtable(L, size, 0);
         for(int i = 0; i < size; ++i) {
            push_form_type_to_stack(L, result[i]);
            lua_seti(L, -2, i + 1);
         }
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t allow_none(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*)self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         bool  value   = lua_toboolean(L, 2);
         task->handler = [widget, value]() { widget->setAllowNone(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t default_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<wrappers::form>(L, 2);
         luaL_argcheck(L, arg, 2, "form expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         auto* value   = arg->stub;
         task->handler = [widget, value]() { widget->setDefaultForm(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg = wrapper_from_stack<wrappers::form>(L, 2);
         luaL_argcheck(L, arg, 2, "form expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*)self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         auto* value   = arg->stub;
         task->handler = [widget, value]() { widget->setFormStub(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t form_types(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 2);
         //
         QVector<dovah::form_type_t> value;
         if (lua_isnumber(L, 2)) {
            bool valid;
            auto ft = get_form_type_from_stack(L, 2, valid);
            luaL_argcheck(L, valid, 2, "form type expected");
            value.push_back(ft);
         } else if (lua_istable(L, 2)) {
            lua_len(L, 2);
            luaL_argcheck(L, lua_isnumber(L, 3), 2, "table argument does not have a numeric length");
            int length = lua_tonumber(L, 3);
            lua_pop(L, 1);
            for (int i = 1; i <= length; ++i) {
               lua_geti(L, 2, i);
               bool valid;
               auto ft = get_form_type_from_stack(L, 3, valid);
               if (valid)
                  value.push_back(ft);
               lua_pop(L, 1);
            }
         } else {
            luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "form type, list of form types, or nil expected");
         }
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*)self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget, value]() { widget->setAllowedFormTypes(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.formpicker.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            created->setAllowedFormTypes(default_form_type_filter);
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         auto* mt = wrap_widget(out, created);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "clear", &_methods::clear },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "allow_none",   &_getters::allow_none },
      { "default_form", &_getters::default_form },
      { "form",         &_getters::form },
      { "form_types",   &_getters::form_types },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "allow_none",   &_setters::allow_none },
      { "default_form", &_setters::default_form },
      { "form",         &_setters::form },
      { "form_types",   &_setters::form_types },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}