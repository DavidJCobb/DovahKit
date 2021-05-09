#include "tab.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include <QSortFilterProxyModel>
#include "../../../ui/util/color.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "_util.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::tabbox_tab;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t tab_enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool found;
         bool result = editor_script::util::tabbox::get_tab_property(self.widget, found, &QTabWidget::isTabEnabled);
         if (!found)
            return 0;
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t tab_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool    found;
         QString result = editor_script::util::tabbox::get_tab_property(self.widget, found, &QTabWidget::tabText);
         if (!found)
            return 0;
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t tab_tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool    found;
         QString result = editor_script::util::tabbox::get_tab_property(self.widget, found, &QTabWidget::tabToolTip);
         if (!found)
            return 0;
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t tab_whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool    found;
         QString result = editor_script::util::tabbox::get_tab_property(self.widget, found, &QTabWidget::tabWhatsThis);
         if (!found)
            return 0;
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t tab_enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         bool value = lua_toboolean(L, 2);
         editor_script::util::tabbox::set_tab_property(self.widget, &QTabWidget::setTabEnabled, value);
         return 0;
      }
      luastackchange_t tab_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QString value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::util::tabbox::set_tab_property(self.widget, &QTabWidget::setTabText, value);
         return 0;
      }
      luastackchange_t tab_tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QString value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::util::tabbox::set_tab_property(self.widget, &QTabWidget::setTabToolTip, value);
         return 0;
      }
      luastackchange_t tab_whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QString value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::util::tabbox::set_tab_property(self.widget, &QTabWidget::setTabWhatsThis, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "tab_enabled",    &_getters::tab_enabled },
      { "tab_name",       &_getters::tab_name },
      { "tab_tooltip",    &_getters::tab_tooltip },
      { "tab_whats_this", &_getters::tab_whats_this },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "tab_enabled",    &_setters::tab_enabled },
      { "tab_name",       &_setters::tab_name },
      { "tab_tooltip",    &_setters::tab_tooltip },
      { "tab_whats_this", &_setters::tab_whats_this },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}