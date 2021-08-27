#include "tab.h"
#include <QTabWidget>
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../wrapper.h"

#include "../../../api_helpers/widget_properties.h"

#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../tasks/s2m/ui_write_lambda.h"

#include "_util.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::tabbox_tab;
   using wrapped_type = cls::wrapped_type;

   namespace _impl {
      using namespace dovahscript::wrappers::ui::impl::tabbox;
   }

   namespace _methods {
   }
   namespace _getters {
      int tab_enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool found;
         bool result = _impl::get_tab_property(self.widget, found, &QTabWidget::isTabEnabled);
         if (!found)
            return 0;
         lua_pushboolean(L, result);
         return 1;
      }
      int tab_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool    found;
         QString result = _impl::get_tab_property(self.widget, found, &QTabWidget::tabText);
         if (!found)
            return 0;
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int tab_tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool    found;
         QString result = _impl::get_tab_property(self.widget, found, &QTabWidget::tabToolTip);
         if (!found)
            return 0;
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int tab_whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool    found;
         QString result = _impl::get_tab_property(self.widget, found, &QTabWidget::tabWhatsThis);
         if (!found)
            return 0;
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int tab_enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         bool value = lua_toboolean(L, 2);
         _impl::set_tab_property(self.widget, &QTabWidget::setTabEnabled, value);
         return 0;
      }
      int tab_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QString value = QString::fromUtf8(lua_tostring(L, 2));
         _impl::set_tab_property(self.widget, &QTabWidget::setTabText, value);
         return 0;
      }
      int tab_tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QString value = QString::fromUtf8(lua_tostring(L, 2));
         _impl::set_tab_property(self.widget, &QTabWidget::setTabToolTip, value);
         return 0;
      }
      int tab_whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QString value = QString::fromUtf8(lua_tostring(L, 2));
         _impl::set_tab_property(self.widget, &QTabWidget::setTabWhatsThis, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::ui {
   /*static*/ cls::method_list_t cls::metatable_methods = {
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "tab_enabled",    &_getters::tab_enabled },
      { "tab_name",       &_getters::tab_name },
      { "tab_tooltip",    &_getters::tab_tooltip },
      { "tab_whats_this", &_getters::tab_whats_this },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "tab_enabled",    &_setters::tab_enabled },
      { "tab_name",       &_setters::tab_name },
      { "tab_tooltip",    &_setters::tab_tooltip },
      { "tab_whats_this", &_setters::tab_whats_this },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}