#include "layer_group.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "layer.h"

namespace {
   using _blend_mode = CanvasWidgetEntity::CompositionMode;
   using _blend_mode_name = std::pair<_blend_mode, const char*>;

   std::array _modes_to_strings = {
      _blend_mode_name{ _blend_mode::CompositionMode_Clear,      "clear" },
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
      _blend_mode_name{ _blend_mode::CompositionMode_Xor,        "xor" },
   };
}

#pragma region Collection: "layers"
namespace {
   using namespace editor_script;

   namespace _collections::layers {
      using cls        = wrappers::ui::canvas_layer_group;
      using group_t    = CanvasWidgetLayerGroup;
      using layer_t    = CanvasWidgetLayer;

      static constexpr auto collection_key = cls::layer_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         if (self->widget == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& group = *(group_t*)self.canvas_layer;
         assert(group.isLayerGroup());
         lua_pushinteger(L, group.childLayers().size());
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         int isnum;
         int i = lua_tointegerx(L, 2, &isnum);
         if (!isnum)
            return 0;
         --i;
         if (i < 0)
            return 0;
         //
         CanvasWidgetEntity* layer = nullptr;
         {
            auto* task  = new tasks::s2m::lambda(true);
            auto* group = (group_t*)self.canvas_layer;
            assert(group->isLayerGroup());
            task->handler = [group, i, &layer]() {
               auto list = group->childLayers();
               if (i >= list.size())
                  return;
               layer = list[i];
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!layer)
            return 0;
         //
         if (layer->isLayerGroup()) {
            wrapper iw;
            iw.type = wrapper_type::ui_canvas_layer;
            iw.canvas_layer = layer;
            return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::canvas_layer_group::metatable_key);
         } else if (layer->isLayer()) {
            wrapper iw;
            iw.type = wrapper_type::ui_canvas_layer;
            iw.canvas_layer = layer;
            return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::canvas_layer::metatable_key);
         }
         return 0;
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::canvas_layer_group;
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
      luastackchange_t layers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_canvas_layers;
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::layer_collection_key);
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
                  lua_pushstring(L, pair.second);
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
      luastackchange_t opacity(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         bool value = lua_tonumber(L, 2);
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
      { "layers",     &_getters::layers },
      { "opacity",    &_getters::opacity },
      { "x",          &_getters::x },
      { "y",          &_getters::y },
      { "visible",    &_getters::visible },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "blend_mode", &_setters::blend_mode },
      { "opacity",    &_getters::opacity },
      { "x",          &_setters::x },
      { "y",          &_setters::y },
      { "visible",    &_setters::visible },
   };
}