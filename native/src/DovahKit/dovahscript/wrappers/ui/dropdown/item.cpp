#include "item.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../wrapper.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::ui::dropdown_item;

   namespace _methods {
   }
   namespace _getters {
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