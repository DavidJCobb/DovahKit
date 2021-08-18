#include "qt_variant.h"
#include "../../helpers/lua/qt_variant.h"
#include "../../lua.h"
#include "../core/classes.h"
#include "../core/verify_threading.h"
#include "../wrappers/base.h"
#include "../wrapper.h"

namespace dovahscript::api_helpers {
   extern QVariant pull_variant(lua_State* L, int stack_pos) {
      core::require_script_thread();
      //
      if (lua_gettop(L) < stack_pos)
         return QVariant();
      if (lua_type(L, stack_pos) != LUA_TUSERDATA)
         return cobb::lua::to_qt_variant(L, stack_pos);
      //
      if (auto* w = (wrapper*) classes::cast_to_class(L, stack_pos, wrapper_metatable::metatable_key)) {
         if (w->depth || w->is_collection)
            return QVariant();
         switch (w->type) {
            case wrapper_type::button_group:
            case wrapper_type::canvas_entity:
            case wrapper_type::canvas_layer_data:
            case wrapper_type::widget:
               return QVariant::fromValue<QObject*>((QObject*)w->pertinent_pointer);
            case wrapper_type::form:
               return QVariant::fromValue<dovah::form_stub*>(w->stub);
         }
         return QVariant();
      }
      //
      return QVariant();
   }
}