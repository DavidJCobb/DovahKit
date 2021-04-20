#include "cell.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include <QSortFilterProxyModel>
#include "../../../ui/util/color.h"
#include "../../../ui/util/lua_item_model.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"
#include "../helpers/model_observer_data.h"

#include "cell.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::table_view_cell;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         int result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               result = observer->col;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return DovahKitScriptVMCore::get().push_to_lua(result + 1); // Lua is one-indexed
      }
      luastackchange_t row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         int result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               result = observer->row;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return DovahKitScriptVMCore::get().push_to_lua(result + 1); // Lua is one-indexed
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QString result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               auto* item = observer->item();
               if (item)
                  result = item->data(Qt::DisplayRole).toString();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t text_color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QVariant result = helpers::get_model_items_data(self.model_observer, Qt::ForegroundRole);
         QColor   color;
         switch (result.type()) {
            case QMetaType::QBrush:
               color = result.value<QBrush>().color();
               break;
            case QMetaType::QColor:
               color = result.value<QColor>();
               break;
            default:
               lua_pushnil(L);
               return 1;
         }
         lua_settop(L, 1);
         util::ui::push_color(L, color);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         QString value = lua_tostring(L, 2);
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::lambda(false);
            task->handler  = [observer, value]() {
               auto* item = observer->item();
               if (item)
                  item->setData(value, Qt::DisplayRole);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
         }
         return 0;
      }
      luastackchange_t text_color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QColor color;
         //
         lua_settop(L, 2);
         switch (lua_type(L, 2)) {
            case LUA_TNONE:
            case LUA_TNIL:
               helpers::set_model_items_data(self.model_observer, Qt::ForegroundRole, QVariant());
               return 0;
            default:
               color = util::ui::pull_color(L, 2);
               break;
         }
         helpers::set_model_items_data(self.model_observer, Qt::ForegroundRole, color);
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
      { "column",     &_getters::column },
      { "row",        &_getters::row },
      { "text",       &_getters::text },
      { "text_color", &_getters::text_color },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "text",       &_setters::text },
      { "text_color", &_setters::text_color },
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