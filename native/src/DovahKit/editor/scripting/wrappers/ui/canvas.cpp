#include "canvas.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"
#include "../../collections.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "helpers/widget_properties.h"

#include "canvas/layer.h"
#include "canvas/layer_group.h"

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
         CanvasWidgetEntity* layer = nullptr;
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
               layer->setVisible(true);
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
      luastackchange_t height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &CanvasWidget::imageHeight);
         lua_pushinteger(L, result);
         return 1;
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
      luastackchange_t width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &CanvasWidget::imageWidth);
         lua_pushinteger(L, result);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "height (integer) expected");
         luaL_argcheck(L, value >= 0, 2, "the size cannot be negative");
         if (!self.widget)
            return 0;
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &CanvasWidget::setImageHeight, value);
         return 0;
      }
      luastackchange_t width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "width (integer) expected");
         luaL_argcheck(L, value >= 0, 2, "the size cannot be negative");
         if (!self.widget)
            return 0;
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &CanvasWidget::setImageWidth, value);
         return 0;
      }
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
      { "height", &_getters::height },
      { "layers", &_getters::layers },
      { "width",  &_getters::width },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "height", &_setters::height },
      { "width",  &_setters::width },
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
      //
      // Set up collection:
      //
      editor_script::define_collection_metatable(L, {
         .registry_key          = cls::layer_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::layers::get_collection_length,
         .lookup_item_by_index   = &_collections::layers::lookup_item_by_index,
      });
   }
}