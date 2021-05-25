#include "item.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include "../../../ui/util/lua_item_model.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "../../../../../helpers/lua/qt_variant.h"

#include "../helpers/model_observer_data.h"

#include "../../resource/raster.h"

#pragma region dropdown_item
namespace {
   using namespace editor_script;
   using cls = wrappers::ui::dropdown_item;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t data(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QVariant result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               if (auto* item = observer->item())
                  result = item->data(Qt::UserRole);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return DovahKitScriptVMCore::get().push_to_lua(result);
      }
      luastackchange_t icon(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         LuaManagedResourceHandle result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               auto* item = observer->item();
               if (!item)
                  return;
               auto  data     = item->data(Qt::DecorationRole);
               auto* resource = LuaManagedResourceHandle::extract_from_variant(data);
               if (!resource)
                  return;
               result = resource;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         return wrappers::resource::raster::wrap_and_push(L, *result);
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
               if (auto* item = observer->item())
                  result = item->data(Qt::DisplayRole).toString();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t data(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         if (lua_type(L, 2) == LUA_TTABLE) {
            luaL_error(L, "storing a table as a dropdown item's data member is not supported");
         }
         auto  value = vm.variant_from_lua(2);
         if (!value.isValid() && !lua_isnoneornil(L, 2)) {
            luaL_error(L, "the provided value cannot be stored as a dropdown item's data member");
         }
         if (!self.model_observer)
            return 0;
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         task->handler  = [observer, value]() {
            if (auto* item = observer->item())
               item->setData(value, Qt::UserRole);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t icon(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         //
         LuaManagedResourceHandle value; // must use a handle here, to avoid race conditions that stem from this being non-blocking (i.e. Lua var goes out of scope, gets closed or GC'd, before we send the resource to Qt)
         if (!lua_isnoneornil(L, 2)) {
            auto* arg = wrapper_from_stack<wrappers::resource::raster>(L, 2);
            luaL_argcheck(L, arg != nullptr, 2, "raster expected");
            value = arg->managed_resource;
         }
         //
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         task->handler  = [observer, value]() mutable {
            if (auto* item = observer->item()) {
               auto wrapped = QVariant::fromValue<LuaManagedResourceHandle>(value);
               item->setData(wrapped, Qt::DecorationRole);
            }
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.model_observer)
            return 0;
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         auto  value    = QString::fromUtf8(lua_tostring(L, 2));
         task->handler  = [observer, value]() {
            if (auto* item = observer->item())
               item->setData(value, Qt::DisplayRole);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
      { "data", &_getters::data }, // an arbitrary scalar value that can be associated with any dropdown item; uses Qt::UserRole
      //
      // For fields that are handled as item-data (i.e. Qt::ItemDataRole), please use the 
      // "model observer property handler" system. A list of MOPHs for this Lua class is 
      // defined below.
      //
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "data",  &_setters::data }, // an arbitrary scalar value that can be associated with any dropdown item; uses Qt::UserRole
      //
      // For fields that are handled as item-data (i.e. Qt::ItemDataRole), please use the 
      // "model observer property handler" system. A list of MOPHs for this Lua class is 
      // defined below.
      //
   };

   /*static*/ const moph::handler_set cls::moph_handlers = {{
      moph::model_observer_property_handler{ "alignment",  Qt::ItemDataRole::TextAlignmentRole, moph::push_alignment, moph::pull_alignment, moph::transform_alignment },
      moph::model_observer_property_handler{ "icon",       Qt::ItemDataRole::DecorationRole,    moph::push_icon,      moph::pull_icon,      moph::model_observer_property_handler::default_transform, true },
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
#pragma endregion