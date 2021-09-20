#include "setting.h"
#include "../../../helpers/qt/ini.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"

#include "../../tasks/s2m/lambda.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::ini::setting;

   namespace _methods {
   }
   namespace _getters {
      int current_value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.game_ini_setting)
            return 0;
         QVariant value;
         QChar    starts = '\0';
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [setting = (const cobb::qt::ini::Setting*)self.game_ini_setting, &value, &starts]() {
               value = setting->currentValue();
               assert(value.isValid());
               if (!setting->name.isEmpty())
                  starts = setting->name[0];
            };
            send_script_task(*task);
            delete task;
         }
         switch (starts.toLower().unicode()) {
            case 'b':
               lua_pushboolean(L, value.value<bool>());
               return 1;
            case 'i':
               lua_pushinteger(L, value.value<int32_t>());
               return 1;
            case 'f':
               lua_pushnumber(L, value.value<float>());
               return 1;
            case 'u':
               lua_pushinteger(L, value.value<uint32_t>());
               return 1;
            case 's':
               lua_pushstring(L, value.value<QString>().toUtf8());
               return 1;
         }
         return 0;
      }
      int default_value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.game_ini_setting)
            return 0;
         QVariant value;
         QChar    starts = '\0';
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [setting = (const cobb::qt::ini::Setting*)self.game_ini_setting, &value, &starts]() {
               value = setting->initialValue();
               if (!setting->name.isEmpty())
                  starts = setting->name[0];
            };
            send_script_task(*task);
            delete task;
         }
         switch (starts.toLower().unicode()) {
            case 'b':
               lua_pushboolean(L, value.value<bool>());
               return 1;
            case 'i':
               lua_pushinteger(L, value.value<int32_t>());
               return 1;
            case 'f':
               lua_pushnumber(L, value.value<float>());
               return 1;
            case 'u':
               lua_pushinteger(L, value.value<uint32_t>());
               return 1;
            case 's':
               lua_pushstring(L, value.value<QString>().toUtf8());
               return 1;
         }
         return 0;
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.game_ini_setting)
            return 0;
         QString value;
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [setting = (const cobb::qt::ini::Setting*)self.game_ini_setting, &value]() {
               value = setting->name;
            };
            send_script_task(*task);
            delete task;
         }
         lua_pushstring(L, value.toUtf8());
         return 1;
      }
   }
   namespace _setters {
   }
}

namespace dovahscript::wrappers::ini {
   /*static*/ cls::method_list_t cls::metatable_methods = {
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "current_value", &_getters::current_value },
      { "default_value", &_getters::default_value },
      { "name",          &_getters::name },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };
}