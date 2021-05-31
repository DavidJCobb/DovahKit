#include "scrollbox.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/alignment.h"

#include "helpers/widget_properties.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::scrollbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t scroll_to(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "x-coordinate (integer) expected");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "y-coordinate (integer) expected");
         int w = lua_tointegerx(L, 4, &isnum);
         if (!isnum || w < 0)
            w = 0;
         int h = lua_tointegerx(L, 5, &isnum);
         if (!isnum || h < 0)
            h = 0;
         //
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [widget, x, y, w, h]() {
               if (w || h)
                  widget->ensureVisible(x + w, y + h);
               widget->ensureVisible(x, y);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t body(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QWidget* result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QScrollArea::widget);
         if (!result)
            return 0;
         wrapper out;
         auto* mt = wrap_widget(out, result);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.scrollbox.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
            //
            auto* body = new QWidget(created);
            //body->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
            body->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            body->setProperty("Lua widget forced parent", QVariant::fromValue<QObject*>(created));
            created->setWidget(body);
            created->setWidgetResizable(true);
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
      { "scroll_to", &_methods::scroll_to },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "body", &_getters::body },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
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