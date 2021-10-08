#include "image_widget.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"

#include "../../api_helpers/widget_properties.h"

#include "../resource/dds.h"
#include "../resource/raster.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::image_widget;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int image(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         task_reference<DovahscriptResource> result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::resource);
         return push_native_object(result);
      }
   }
   namespace _setters {
      int image(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         task_reference<DovahscriptResource> handle;
         if (auto* wrap = wrapper_from_stack<wrappers::resource::dds>(L, 2)) {
            handle = wrap->managed_resource;
         } else if (auto* wrap = wrapper_from_stack<wrappers::resource::raster>(L, 2)) {
            handle = wrap->managed_resource;
         }
         if (!handle) {
            luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "raster, dds_resource, or nil expected");
         }
         //
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setResource, handle);
         return 0;
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
      { "image", &_getters::image },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "image", &_setters::image },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}