#include "file_save_button.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/resources.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"

#include "../../api_helpers/qt_color.h"
#include "../../api_helpers/widget_properties.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::file_save_button;
   using wrapped_type = cls::wrapped_type;

   using DSRH = DovahscriptResourceHandle;

   namespace _methods {
   }
   namespace _getters {
      int data(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         DSRH    resource;
         QString text;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [&resource, &text, widget = task_reference((wrapped_type*)self.widget)]() {
               resource = widget->resourceContent();
               text     = widget->textContent();
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (text.isEmpty()) {
            return push_native_object(resource);
         } else {
            const auto data = text.toUtf8();
            lua_pushlstring(L, data.constData(), data.size());
            return 1;
         }
      }
      int filename(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::desiredFilename);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int label(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int data(lua_State* L) {
         auto&   self = get_wrapper_for_thiscall<cls>(L);
         DSRH    resource;
         QString text;
         if (!lua_isnoneornil(L, 2)) {
            if (lua_isstring(L, 2)) {
               size_t size;
               auto*  data = lua_tolstring(L, 2, &size);
               text = QString(QByteArray(data, size));
            } else {
               wrapper* arg = wrapper_from_stack<wrapper_metatable>(L, 2);
               cobb::lua::argcheck(L, arg != nullptr, 2, "resource object expected");
               cobb::lua::argcheck(L, arg->type == wrapper_type::lua_managed_resource, 2, "resource object expected");
               resource = arg->managed_resource;
            }
         }
         if (!self.widget)
            return 0;
         auto* task = new tasks::s2m::ui_write_lambda_ex(false, [resource, text, widget = task_reference((wrapped_type*)self.widget)]() {
            if (text.isEmpty()) {
               widget->setContent(resource);
            } else {
               widget->setContent(text);
            }
         });
         send_script_ui_task(*task);
         return 0;
      }
      int filename(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         QString value;
         if (!lua_isnoneornil(L, 2)) {
            cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "text (label) expected");
            value = QString::fromUtf8(lua_tostring(L, 2));
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setDesiredFilename, value);
      }
      int label(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         QString value;
         if (!lua_isnoneornil(L, 2)) {
            cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "text (label) expected");
            value = QString::fromUtf8(lua_tostring(L, 2));
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setText, value);
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
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
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "data",     &_getters::data },
      { "filename", &_getters::filename },
      { "label",    &_getters::label },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "data",     &_setters::data },
      { "filename", &_setters::filename },
      { "label",    &_setters::label },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}