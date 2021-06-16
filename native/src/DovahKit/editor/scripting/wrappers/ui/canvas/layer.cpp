#include "layer.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "../../../widgets/objects/CanvasWidgetLayerDataLuaManagedResource.h"
#include "../../resource/dds.h"
#include "../../resource/raster.h"

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
   using namespace editor_script;
   using cls = wrappers::ui::canvas_layer;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t blend_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         CanvasWidgetEntity::CompositionMode result;
         {
            auto* layer   = self.canvas_layer;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->compositionMode();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
      luastackchange_t canvas(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         assert(self.canvas_layer->isLayer());
         CanvasWidget* result = nullptr;
         {
            auto* layer   = self.canvas_layer;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->canvas();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         wrapper out;
         auto* mt = wrap_widget(out, result);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t data(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         assert(self.canvas_layer->isLayer());
         auto* layer = (CanvasWidgetLayer*) self.canvas_layer;
         //
         CanvasWidgetLayerData* result = nullptr;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->data();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         if (auto* res_layer = qobject_cast<CanvasWidgetLayerDataLuaManagedResource*>(result)) {
            //
            // If the layer-data type is CanvasWidgetLayerDataLuaManagedResource, then the "canvas layer 
            // data" object should be invisible to the script; as far as the script is concerned, the 
            // layer data is the wrapped Lua-managed resource and not the CWLDLMR wrapping it.
            //
            LuaManagedResourceHandle handle = res_layer->resource();
            if (!handle)
               return 0;
            switch (handle->resource_type()) {
               using t = editor_script::lua_managed_resource_type;
               case t::dds:
                  return wrappers::resource::dds::wrap_and_push(L, *handle);
               case t::raster:
                  return wrappers::resource::raster::wrap_and_push(L, *handle);
            }
            return 0;
         }
         return 0;
      }
      luastackchange_t opacity(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         lua_Number result;
         {
            auto* layer   = self.canvas_layer;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->opacity();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushnumber(L, result);
         return 1;
      }
      luastackchange_t visible(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         bool result;
         {
            auto* layer   = self.canvas_layer;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->visible();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         int result;
         {
            auto* layer   = self.canvas_layer;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->position().x();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      luastackchange_t y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
         int result;
         {
            auto* layer   = self.canvas_layer;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer, &result]() {
               result = layer->position().y();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t blend_mode(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* name  = lua_tostring(L, 2);
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
         if (!self.canvas_layer)
            return 0;
         auto* layer   = self.canvas_layer;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            layer->setCompositionMode(value);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t data(lua_State* L) {
         auto&    self = get_wrapper_for_thiscall<cls>(L);
         QVariant value;
         if (auto* wrap = wrapper_from_stack<wrappers::resource::dds>(L, 2)) {
            if (wrap->managed_resource)
               value = QVariant::fromValue<LuaManagedResourceHandle>(wrap->managed_resource);
         } else if (auto* wrap = wrapper_from_stack<wrappers::resource::raster>(L, 2)) {
            if (wrap->managed_resource)
               value = QVariant::fromValue<LuaManagedResourceHandle>(wrap->managed_resource);
         } else {
            //
            // Handling for any new layer data types goes here
            //
            if (!lua_isnoneornil(L, 2)) {
               luaL_argerror(L, 2, "dds_resource, raster, or nil expected");
            }
         }
         if (!self.canvas_layer)
            return 0;
         assert(self.canvas_layer->isLayer());
         auto* layer = (CanvasWidgetLayer*)self.canvas_layer;
         //
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            if (!value.isValid()) {
               layer->setData(nullptr);
               return;
            }
            if (value.userType() == qMetaTypeId<LuaManagedResourceHandle>()) {
               auto* resource = LuaManagedResourceHandle::extract_from_variant(value);
               assert(resource);
               auto* data = new CanvasWidgetLayerDataLuaManagedResource;
               DovahKitScriptVMCore::get().set_up_new_canvas_layer_data(data);
               data->setResource(resource);
               layer->setData(data);
               return;
            }
            //
            // Handling for any new layer data types goes here
            //
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t opacity(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         lua_Number value = lua_tonumber(L, 2);
         if (!self.canvas_layer)
            return 0;
         auto* layer   = self.canvas_layer;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            layer->setOpacity(value);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t visible(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         bool value = lua_toboolean(L, 2);
         if (!self.canvas_layer)
            return 0;
         auto* layer   = self.canvas_layer;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            layer->setVisible(value);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t x(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "x-position (integer) expected");
         if (!self.canvas_layer)
            return 0;
         auto* layer   = self.canvas_layer;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            auto p = layer->position();
            p.setX(value);
            layer->setPosition(p);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t y(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "y-position (integer) expected");
         if (!self.canvas_layer)
            return 0;
         auto* layer   = self.canvas_layer;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            auto p = layer->position();
            p.setY(value);
            layer->setPosition(p);
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
      { "blend_mode", &_getters::blend_mode },
      { "canvas",     &_getters::canvas },
      { "data",       &_getters::data },
      { "opacity",    &_getters::opacity },
      { "x",          &_getters::x },
      { "y",          &_getters::y },
      { "visible",    &_getters::visible },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "blend_mode", &_setters::blend_mode },
      { "data",       &_setters::data },
      { "opacity",    &_setters::opacity },
      { "x",          &_setters::x },
      { "y",          &_setters::y },
      { "visible",    &_setters::visible },
   };
}