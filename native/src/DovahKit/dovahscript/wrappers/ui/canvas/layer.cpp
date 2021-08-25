#include "layer.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/lifetime.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/resources.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/collections.h"
#include "../../../push_native_object.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"

#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../tasks/s2m/ui_write_lambda.h"

#include "../../../qt/DovahscriptCanvasWidgetLayerDataResource.h"
#include "../../../qt/DovahscriptCanvasWidgetLayerDataText.h"
#include "../../resource/dds.h"
#include "../../resource/raster.h"
#include "canvas_text_data.h"

namespace {
   using _blend_mode = CanvasWidgetEntity::CompositionMode;
   using _blend_mode_name = std::pair<_blend_mode, const char*>;

   std::array _modes_to_strings = {
      _blend_mode_name{ _blend_mode::CompositionMode_ColorBurn,  "burn" },
      _blend_mode_name{ _blend_mode::CompositionMode_ColorDodge, "dodge" },
      _blend_mode_name{ _blend_mode::CompositionMode_Darken,     "darken only" },
      _blend_mode_name{ _blend_mode::CompositionMode_Difference, "difference" },
      _blend_mode_name{ _blend_mode::CompositionMode_HardLight,  "hard light" },
      _blend_mode_name{ _blend_mode::CompositionMode_Lighten,    "lighten only" },
      _blend_mode_name{ _blend_mode::CompositionMode_Multiply,   "multiply" },
      _blend_mode_name{ _blend_mode::CompositionMode_Overlay,    "overlay" },
      _blend_mode_name{ _blend_mode::CompositionMode_Plus,       "add" },
      _blend_mode_name{ _blend_mode::CompositionMode_Screen,     "screen" },
      _blend_mode_name{ _blend_mode::CompositionMode_SoftLight,  "soft light" },
      _blend_mode_name{ _blend_mode::CompositionMode_SourceOver, "normal" },
   };
}

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::canvas_layer;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int blend_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         CanvasWidgetEntity::CompositionMode result;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->compositionMode();
            };
            send_script_ui_task(*task);
            delete task;
         }
         for (auto& pair : _modes_to_strings) {
            if (result == pair.first) {
               lua_pushstring(L, pair.second);
               return 1;
            }
         }
         return 0;
      }
      int canvas(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         assert(self.canvas_entity->isLayer());
         CanvasWidget* result = nullptr;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->canvas();
            };
            send_script_ui_task(*task);
            delete task;
         }
         return push_native_object(result);
      }
      int data(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         assert(self.canvas_entity->isLayer());
         //
         CanvasWidgetLayerData* result = nullptr;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  layer   = task_reference((CanvasWidgetLayer*) self.canvas_entity);
            task->handler = [layer, &result]() {
               result = layer->data();
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (!result)
            return 0;
         if (auto* res_layer = qobject_cast<DovahscriptCanvasWidgetLayerDataResource*>(result)) {
            //
            // If the layer-data type is CanvasWidgetLayerDataLuaManagedResource, then the "canvas layer 
            // data" object should be invisible to the script; as far as the script is concerned, the 
            // layer data is the wrapped Lua-managed resource and not the CWLDLMR wrapping it.
            //
            DovahscriptResourceHandle handle = res_layer->resource();
            return push_native_object(handle);
         }
         if (auto* text_layer = qobject_cast<DovahscriptCanvasWidgetLayerDataText*>(result)) {
            return push_native_object(text_layer);
         }
         return 0;
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         QString result;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->objectName();
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int opacity(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         lua_Number result;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->opacity();
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushnumber(L, result);
         return 1;
      }
      int visible(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         bool result;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->visible();
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      int x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         int result;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->position().x();
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         int result;
         {
            auto  layer   = task_reference(self.canvas_entity);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->position().y();
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
   }
   namespace _setters {
      int blend_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* name = lua_tostring(L, 2);
         //
         _blend_mode value;
         {
            bool found = false;
            for (auto& pair : _modes_to_strings) {
               if (_stricmp(name, pair.second) == 0) {
                  value = pair.first;
                  found = true;
                  break;
               }
            }
            luaL_argcheck(L, found, 2, "the specified name is not a recognized blend mode");
         }
         if (!self.canvas_entity)
            return 0;
         auto  layer   = task_reference(self.canvas_entity);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [layer, value]() {
            layer->setCompositionMode(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int data(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         auto pointer  = task_reference<DovahscriptCanvasWidgetLayerData>(nullptr);
         auto resource = DovahscriptResourceHandle();
         //
         if (auto* wrap = wrapper_from_stack<wrappers::resource::dds>(L, 2)) {
            if (wrap->managed_resource)
               resource = wrap->managed_resource;
         } else if (auto* wrap = wrapper_from_stack<wrappers::resource::raster>(L, 2)) {
            if (wrap->managed_resource)
               resource = wrap->managed_resource;
         } else if (auto* wrap = wrapper_from_stack<wrappers::ui::canvas_text_data>(L, 2)) {
            if (wrap->canvas_layer_data)
               pointer  = wrap->canvas_layer_data;
         } else {
            if (!lua_isnoneornil(L, 2)) {
               cobb::lua::argerror(L, 2, "dds_resource, raster, or nil expected");
            }
         }
         if (!self.canvas_entity)
            return 0;
         assert(self.canvas_entity->isLayer());
         //
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto  layer   = task_reference((CanvasWidgetLayer*) self.canvas_entity);
         task->handler = [layer, pointer, resource]() {
            if (!pointer && !resource) {
               layer->setData(nullptr);
               return;
            }
            if (resource) {
               auto* data = qobject_cast<DovahscriptCanvasWidgetLayerDataResource*>(layer->data());
               if (!data) {
                  data = new DovahscriptCanvasWidgetLayerDataResource;
                  core::subsystems::lifetime::get().on_non_hierarchy_object_created(*data);
               }
               data->setResource(resource);
               layer->setData(data);
               return;
            }
            if (pointer) {
               layer->setData(pointer);
               return;
            }
         };
         send_script_ui_task(*task);
         return 0;
      }
      int name(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* value = lua_tostring(L, 2);
         if (!self.canvas_entity)
            return 0;
         auto  layer   = task_reference(self.canvas_entity);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [layer, value]() {
            layer->setObjectName(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int opacity(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         lua_Number value = lua_tonumber(L, 2);
         luaL_argcheck(L, value >= 0.0, 2, "opacity cannot be negative");
         luaL_argcheck(L, value <= 1.0, 2, "opacity cannot exceed 1.0");
         if (!self.canvas_entity)
            return 0;
         auto  layer   = task_reference(self.canvas_entity);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [layer, value]() {
            layer->setOpacity(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int visible(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         bool  value = lua_toboolean(L, 2);
         if (!self.canvas_entity)
            return 0;
         auto  layer   = task_reference(self.canvas_entity);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [layer, value]() {
            layer->setVisible(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int x(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "x-position (integer) expected");
         if (!self.canvas_entity)
            return 0;
         auto  layer   = task_reference(self.canvas_entity);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [layer, value]() {
            auto p = layer->position();
            p.setX(value);
            layer->setPosition(p);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int y(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "y-position (integer) expected");
         if (!self.canvas_entity)
            return 0;
         auto  layer   = task_reference(self.canvas_entity);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [layer, value]() {
            auto p = layer->position();
            p.setY(value);
            layer->setPosition(p);
         };
         send_script_ui_task(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
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
      { "blend_mode", &_getters::blend_mode },
      { "canvas",     &_getters::canvas },
      { "data",       &_getters::data },
      { "name",       &_getters::name },
      { "opacity",    &_getters::opacity },
      { "x",          &_getters::x },
      { "y",          &_getters::y },
      { "visible",    &_getters::visible },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "blend_mode", &_setters::blend_mode },
      { "data",       &_setters::data },
      { "name",       &_setters::name },
      { "opacity",    &_setters::opacity },
      { "x",          &_setters::x },
      { "y",          &_setters::y },
      { "visible",    &_setters::visible },
   };
}