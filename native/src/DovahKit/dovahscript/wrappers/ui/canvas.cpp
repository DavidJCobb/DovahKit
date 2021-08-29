#include "canvas.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/lifetime.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_write_lambda.h"
#include "../../tasks/s2m/ui_write_lambda_ex.h"

#include "../../api_helpers/widget_properties.h"

#include "canvas/collection_layers.h"
#include "canvas/layer.h"
#include "canvas/layer_group.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::canvas;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int append_layer(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         CanvasWidgetLayer* layer = nullptr;
         auto* task = new tasks::s2m::ui_write_lambda_ex(true, [&layer, widget = task_reference((wrapped_type*)self.widget)]() {
            layer = widget->createLayer();
            layer->setVisible(true);
            core::subsystems::lifetime::get().on_hierarchy_item_created(*layer);
         });
         send_script_ui_task(*task);
         delete task;
         return push_native_object(layer);
      }
      int append_layer_group(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         CanvasWidgetLayerGroup* layer = nullptr;
         auto* task = new tasks::s2m::ui_write_lambda_ex(true, [&layer, widget = task_reference((wrapped_type*)self.widget)]() {
            layer = widget->createLayerGroup();
            layer->setVisible(true);
            core::subsystems::lifetime::get().on_hierarchy_item_created(*layer);
         });
         send_script_ui_task(*task);
         delete task;
         return push_native_object(layer);
      }
      int remove_layer(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<cls>(L);
         auto  parent = (wrapped_type*) self.widget;
         auto  orphan = (CanvasWidgetEntity*) nullptr;
         int   index  = -1;
         //
         if (lua_isnumber(L, 2)) {
            int isnum;
            index = lua_tointegerx(L, 2, &isnum);
            cobb::lua::argcheck(L, isnum,     2, "integer or userdata expected");
            cobb::lua::argcheck(L, index > 0, 2, "layer indices are numbered from 1");
            --index;
         } else {
            cobb::lua::argcheck(L, lua_isuserdata(L, 2), 2, "integer or userdata expected");
            if (auto* arg = wrapper_from_stack<wrappers::ui::canvas_layer_group>(L, 2))
               orphan = arg->canvas_entity;
            else if (auto* arg = wrapper_from_stack<wrappers::ui::canvas_layer>(L, 2))
               orphan = arg->canvas_entity;
         }
         //
         if (!parent || !orphan)
            return 0;
         //
         int  group_child_count = -1;
         bool is_wrong_parent   = false;
         {
            auto* task    = new tasks::s2m::ui_write_lambda(true);
            task->handler = [parent = task_reference(parent), orphan = task_reference(orphan), index, &group_child_count, &is_wrong_parent]() {
               CanvasWidgetEntity* child_to_remove = orphan;
               if (child_to_remove) {
                  is_wrong_parent = child_to_remove->parent() != parent;
                  if (is_wrong_parent)
                     return;
               } else {
                  assert(index >= 0);
                  auto list = parent->layers();
                  group_child_count = list.size();
                  if (index >= group_child_count)
                     return;
                  child_to_remove = list[index];
               }
               child_to_remove->setParent(nullptr);
               core::subsystems::lifetime::get().on_hierarchy_item_parent_changed(child_to_remove, parent);
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (is_wrong_parent)
            cobb::lua::error(L, "the specified child layer isn't actually a child of the specified canvas");
         if (index >= 0 && index >= group_child_count)
            cobb::lua::error(L, "you cannot remove child layer #%d from a canvas with only %d child layers", index + 1, group_child_count);
         return 0;
      }
   }
   namespace _getters {
      int height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &CanvasWidget::imageHeight);
         lua_pushinteger(L, result);
         return 1;
      }
      int layers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_canvas_layers;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::collections::canvas_layers.registry_key);
      }
      int width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &CanvasWidget::imageWidth);
         lua_pushinteger(L, result);
         return 1;
      }
   }
   namespace _setters {
      int height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "height (integer) expected");
         luaL_argcheck(L, value >= 0, 2, "the size cannot be negative");
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &CanvasWidget::setImageHeight, value);
         return 0;
      }
      int width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "width (integer) expected");
         luaL_argcheck(L, value >= 0, 2, "the size cannot be negative");
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &CanvasWidget::setImageWidth, value);
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
      { "append_layer",       &_methods::append_layer },
      { "append_layer_group", &_methods::append_layer_group },
      { "remove_layer",       &_methods::remove_layer },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "height", &_getters::height },
      { "layers", &_getters::layers },
      { "width",  &_getters::width },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "height", &_setters::height },
      { "width",  &_setters::width },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, collections::canvas_layers);
   }
   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}