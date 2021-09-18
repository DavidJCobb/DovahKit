#include "cell.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/lua/qt_variant.h"
#include "../../../../helpers/qt/layout.h"
#include "../../../../ui/generic/ObservableStandardItemModel.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"
#include "../../../widget_overrides.h"
#include "../../../wrapper.h"

#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../tasks/s2m/ui_write_lambda.h"

#include "../../../api_helpers/model_observers.h"
#include "../../../api_helpers/model_observer_property_handlers.h"
#include "../../../api_helpers/widget_properties.h"
#include "../../../api_helpers/qt_alignment.h"
#include "../../../api_helpers/qt_color.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::ui::table_view_cell;

   namespace _methods {
   }
   namespace _getters {
      int column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         int result;
         {
            auto  observer = task_reference(self.model_observer);
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               result = observer->col;
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushinteger(L, result + 1); // Lua is one-indexed
         return 1;
      }
      int row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         int result;
         {
            auto  observer = task_reference(self.model_observer);
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               result = observer->row;
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushinteger(L, result + 1); // Lua is one-indexed
         return 1;
      }
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace {
   namespace moph {
      using namespace api_helpers::moph;
   }
}
namespace dovahscript::wrappers::ui {
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
      moph::model_observer_property_handler{ "font",       Qt::ItemDataRole::FontRole,          moph::push_font,      moph::pull_font,      moph::model_observer_property_handler::default_transform, true },
      moph::model_observer_property_handler{ "icon",       Qt::ItemDataRole::DecorationRole,    moph::push_icon,      moph::pull_icon,      moph::model_observer_property_handler::default_transform, true },
      moph::model_observer_property_handler{ "text",       Qt::ItemDataRole::DisplayRole,       moph::push_string,    moph::pull_string },
      moph::model_observer_property_handler{ "text_color", Qt::ItemDataRole::ForegroundRole,    moph::push_color,     moph::pull_color,     moph::model_observer_property_handler::default_transform, true },
   }};

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      int index_class   = lua_absindex(L, -3);
      int index_getters = lua_absindex(L, -2);
      int index_setters = lua_absindex(L, -1);
      //
      cls::moph_handlers.extend(L, cls::metatable_key, index_getters, index_setters);
   }

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}