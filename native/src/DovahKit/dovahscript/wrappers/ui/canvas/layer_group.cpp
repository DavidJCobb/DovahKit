#include "layer_group.h"
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

#include "../../../api_helpers/widget_properties.h"

#include "../canvas.h"
#include "layer.h"
#include "layer_group/collection_layers.h"

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
   using cls          = wrappers::ui::canvas_layer_group;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int append_layer(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         assert(self.canvas_entity->isLayerGroup());
         CanvasWidgetLayer* layer = nullptr;
         {
            auto* task    = new tasks::s2m::ui_write_lambda(true);
            auto  group   = task_reference((CanvasWidgetLayerGroup*)self.canvas_entity);
            task->handler = [group, &layer]() {
               layer = group->createLayer();
               layer->setVisible(true);
               core::subsystems::lifetime::get().on_hierarchy_item_created(*layer);
            };
            send_script_ui_task(*task);
            delete task;
         }
         return push_native_object(layer);
      }
      int append_layer_group(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         assert(self.canvas_entity->isLayerGroup());
         CanvasWidgetLayerGroup* layer = nullptr;
         {
            auto* task    = new tasks::s2m::ui_write_lambda(true);
            auto  group   = task_reference((CanvasWidgetLayerGroup*)self.canvas_entity);
            task->handler = [group, &layer]() {
               layer = group->createLayerGroup();
               layer->setVisible(true);
               core::subsystems::lifetime::get().on_hierarchy_item_created(*layer);
            };
            send_script_ui_task(*task);
            delete task;
         }
         return push_native_object(layer);
      }
      int remove_layer(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<cls>(L);
         auto  parent = task_reference((wrapped_type*) self.canvas_entity);
         auto  orphan = task_reference<CanvasWidgetEntity>(nullptr);
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
            if (auto* arg = wrapper_from_stack<cls>(L, 2))
               orphan = arg->canvas_entity;
            else if (auto* arg = wrapper_from_stack<wrappers::ui::canvas_layer>(L, 2))
               orphan = arg->canvas_entity;
         }
         //
         if (!parent || !orphan)
            return 0;
         assert(parent->isLayerGroup());
         //
         int group_child_count = -1;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [parent, orphan, index, &group_child_count]() {
               CanvasWidgetEntity* child_to_remove = orphan;
               if (!orphan) {
                  assert(index >= 0);
                  auto list = parent->childLayers();
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
         if (index >= 0 && index >= group_child_count) {
            cobb::lua::error(L, "you cannot remove child #%d from a layer group with only %d children", index + 1, group_child_count);
         }
         return 0;
      }
   }
   namespace _getters {
      int blend_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         CanvasWidgetEntity::CompositionMode result = api_helpers::get_widget_property(self.canvas_entity, &CanvasWidgetEntity::compositionMode);
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
         CanvasWidget* result = api_helpers::get_widget_property(self.canvas_entity, &CanvasWidgetEntity::canvas);
         return push_native_object(result);
      }
      int layers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_canvas_layers;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::collections::canvas_layer_group_children.registry_key);
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         QString result = api_helpers::get_widget_property(self.canvas_entity, &QObject::objectName);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int opacity(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         lua_Number result = api_helpers::get_widget_property(self.canvas_entity, &CanvasWidgetEntity::opacity);
         lua_pushnumber(L, result);
         return 1;
      }
      int visible(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         bool result = api_helpers::get_widget_property(self.canvas_entity, &CanvasWidgetEntity::visible);
         lua_pushboolean(L, result);
         return 1;
      }
      int x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.canvas_entity)
            return 0;
         int result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer = task_reference(self.canvas_entity), &result]() {
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
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [layer = task_reference(self.canvas_entity), &result]() {
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
            cobb::lua::argcheck(L, found, 2, "the specified name is not a recognized blend mode");
         }
         if (!self.canvas_entity)
            return 0;
         api_helpers::set_widget_property(self.canvas_entity, &CanvasWidgetEntity::setCompositionMode, value);
         return 0;
      }
      int name(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.canvas_entity)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property(self.canvas_entity, &QObject::setObjectName, value);
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
         api_helpers::set_widget_property(self.canvas_entity, &CanvasWidgetEntity::setOpacity, value);
         return 0;
      }
      int visible(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!self.canvas_entity)
            return 0;
         api_helpers::set_widget_property(self.canvas_entity, &CanvasWidgetEntity::setVisible, value);
         return 0;
      }
      int x(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "x-position (integer) expected");
         if (!self.canvas_entity)
            return 0;
         auto* task = new tasks::s2m::ui_write_lambda_ex(false, [value, layer = task_reference(self.canvas_entity)]() {
            auto p = layer->position();
            p.setX(value);
            layer->setPosition(p);
         });
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
         auto* task = new tasks::s2m::ui_write_lambda_ex(false, [value, layer = task_reference(self.canvas_entity)]() {
            auto p = layer->position();
            p.setY(value);
            layer->setPosition(p);
         });
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
      { "append_layer",       &_methods::append_layer },
      { "append_layer_group", &_methods::append_layer_group },
      { "remove_layer",       &_methods::remove_layer },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "blend_mode", &_getters::blend_mode },
      { "canvas",     &_getters::canvas },
      { "layers",     &_getters::layers },
      { "name",       &_getters::name },
      { "opacity",    &_getters::opacity },
      { "x",          &_getters::x },
      { "y",          &_getters::y },
      { "visible",    &_getters::visible },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "blend_mode", &_setters::blend_mode },
      { "name",       &_setters::name },
      { "opacity",    &_setters::opacity },
      { "x",          &_setters::x },
      { "y",          &_setters::y },
      { "visible",    &_setters::visible },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, collections::canvas_layer_group_children);
   }
}