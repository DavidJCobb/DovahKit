#include "formpicker.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/widget_properties.h"

#include "../../lua_libraries/form_types.h"
#include "../form/form.h"

namespace {
   // Default form types to limit new FormPicker widgets to. These are the types 
   // allowed by the normal GetIsID condition function.
   constexpr std::initializer_list<dovah::form_type> default_form_type_filter = {
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
   using namespace dovahscript;
   using cls          = wrappers::ui::formpicker;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->clear(); };
         send_script_ui_task(*task);
         return 0;
      }
   }
   namespace _getters {
      int allow_none(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &FormPicker::allowNone);
         lua_pushboolean(L, result);
         return 1;
      }
      int default_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         dovah::form_stub* result = api_helpers::get_widget_property((wrapped_type*)self.widget, &FormPicker::defaultForm);
         return push_native_object(result);
      }
      int form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         dovah::form_stub* result = api_helpers::get_widget_property((wrapped_type*)self.widget, &FormPicker::formStub);
         return push_native_object(result);
      }
      int form_types(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QVector<dovah::form_type> result = api_helpers::get_widget_property((wrapped_type*)self.widget, &FormPicker::allowedFormTypes);
         int size = result.size();
         lua_createtable(L, size, 0);
         for(int i = 0; i < size; ++i) {
            lua_libraries::form_types::push(L, result[i]);
            lua_seti(L, -2, i + 1);
         }
         return 1;
      }
   }
   namespace _setters {
      int allow_none(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         bool value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &FormPicker::setAllowNone, value);
         return 0;
      }
      int default_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         dovah::form_stub* value = nullptr;
         if (!lua_isnoneornil(L, 2)) {
            auto* arg = wrapper_from_stack<wrappers::form>(L, 2);
            cobb::lua::argcheck(L, arg, 2, "form expected");
            value = arg->stub;
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &FormPicker::setDefaultForm, value);
         return 0;
      }
      int form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         dovah::form_stub* value = nullptr;
         if (!lua_isnoneornil(L, 2)) {
            auto* arg = wrapper_from_stack<wrappers::form>(L, 2);
            cobb::lua::argcheck(L, arg, 2, "form expected");
            value = arg->stub;
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &FormPicker::setFormStub, value);
         return 0;
      }
      int form_types(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 2);
         //
         QVector<dovah::form_type> value;
         bool valid;
         auto ft = lua_libraries::form_types::pull(L, 2, valid);
         if (valid) {
            value.push_back(ft);
         } else if (lua_istable(L, 2)) {
            lua_len(L, 2);
            cobb::lua::argcheck(L, lua_isnumber(L, 3), 2, "table argument does not have a numeric length");
            int length = lua_tonumber(L, 3);
            lua_pop(L, 1);
            for (int i = 1; i <= length; ++i) {
               lua_geti(L, 2, i);
               auto ft = lua_libraries::form_types::pull(L, 3, valid);
               if (valid)
                  value.push_back(ft);
               lua_pop(L, 1);
            }
         } else {
            cobb::lua::argcheck(L, lua_isnoneornil(L, 2), 2, "form type, list of form types, or nil expected");
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &FormPicker::setAllowedFormTypes, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [](wrapped_type* created) {
            created->setAllowNone(true);
            created->setAllowedFormTypes(default_form_type_filter);
         };
         send_script_ui_task(*task);
         auto* created = task->created;
         delete task;
         //
         return push_native_object(created);
      }
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::ui {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "clear", &_methods::clear },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "allow_none",   &_getters::allow_none },
      { "default_form", &_getters::default_form },
      { "form",         &_getters::form },
      { "form_types",   &_getters::form_types },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "allow_none",   &_setters::allow_none },
      { "default_form", &_setters::default_form },
      { "form",         &_setters::form },
      { "form_types",   &_setters::form_types },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}