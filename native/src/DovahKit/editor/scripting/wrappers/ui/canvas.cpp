#include "canvas.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "canvas/layer.h"

#pragma region Collection: "layers"
namespace {
   using namespace editor_script;

   namespace _collections::layers {
      using cls        = wrappers::ui::canvas;
      using widget_t   = CanvasWidget;
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
         auto& self   = get_collection_wrapper(L);
         auto& widget = *(widget_t*)self.widget;
         lua_pushinteger(L, widget.layers().size());
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
         layer_t* layer = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            auto* widget  = (widget_t*)self.widget;
            task->handler = [widget, i, &layer]() {
               auto list = widget->layers();
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
         wrapper iw;
         iw.type = wrapper_type::ui_canvas_layer;
         iw.canvas_layer = layer;
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::canvas_layer::metatable_key);
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::canvas;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t append_layer(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         CanvasWidgetLayer* layer = nullptr;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [widget, &layer]() {
               layer = widget->createLayer();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!layer)
            return 0;
         //
         wrapper iw;
         iw.type = wrapper_type::ui_canvas_layer;
         iw.canvas_layer = layer;
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::canvas_layer::metatable_key);
      }
   }
   namespace _getters {
      luastackchange_t layers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_canvas_layers;
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::layer_collection_key);
      }
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.canvas.new function should not be called with a colon or passed any arguments");
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         auto* mt = wrap_widget(out, created);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "append_layer", &_methods::append_layer },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "layers", &_getters::layers },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}