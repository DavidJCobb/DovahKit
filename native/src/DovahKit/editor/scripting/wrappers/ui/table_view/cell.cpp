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

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::table_view_cell;
   using wrapped_type = cls::wrapped_type;

   namespace _moph {
      using moph_t = moph::model_observer_property_handler;
      
   }

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
   }
   namespace _setters {
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
      { "column", &_getters::column },
      { "row",    &_getters::row },
      //
      // For fields that are handled as item-data (i.e. Qt::ItemDataRole), please use the 
      // "model observer property handler" system. A list of MOPHs for this Lua class is 
      // defined below.
      //
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      //
      // For fields that are handled as item-data (i.e. Qt::ItemDataRole), please use the 
      // "model observer property handler" system. A list of MOPHs for this Lua class is 
      // defined below.
      //
   };

   /*static*/ const moph::handler_set cls::moph_handlers = {{
      moph::model_observer_property_handler{ "alignment",  Qt::ItemDataRole::TextAlignmentRole, moph::push_alignment, moph::pull_alignment, moph::transform_alignment },
      moph::model_observer_property_handler{ "text",       Qt::ItemDataRole::DisplayRole,       moph::push_string,    moph::pull_string },
      moph::model_observer_property_handler{ "text_color", Qt::ItemDataRole::ForegroundRole,    moph::push_color,     moph::pull_color,     moph::model_observer_property_handler::default_transform, true },
   }};

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      int index_class   = lua_absindex(L, -3);
      int index_getters = lua_absindex(L, -2);
      int index_setters = lua_absindex(L, -1);
      //
      cls::moph_handlers.extend(L, cls::metatable_key, index_getters, index_setters);
   }

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