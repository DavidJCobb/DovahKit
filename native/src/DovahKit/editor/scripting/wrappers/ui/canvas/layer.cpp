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
   using namespace editor_script;
   using cls = wrappers::ui::canvas_layer;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t canvas(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_layer)
            return 0;
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
         CanvasWidgetLayerData* result = nullptr;
         {
            auto* layer   = self.canvas_layer;
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
            luaL_argerror(L, 2, "dds_resource or raster expected");
         }
         if (!self.canvas_layer)
            return 0;
         auto* layer   = self.canvas_layer;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [layer, value]() {
            if (value.type() == qMetaTypeId<LuaManagedResourceHandle>()) {
               auto* resource = LuaManagedResourceHandle::extract_from_variant(value);
               assert(resource);

               static_assert(false, "THIS IS BAD! The VM core needs to keep track of all scripted CanvasWidgetLayerData objects so that it can delete them as appropriate during VM teardown!");

               auto* data = new CanvasWidgetLayerDataLuaManagedResource;
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
      { "canvas", &_getters::canvas },
      { "data",   &_getters::data },
      { "x",      &_getters::x },
      { "y",      &_getters::y },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "data", &_setters::data },
      { "x",    &_setters::x },
      { "y",    &_setters::y },
   };
}