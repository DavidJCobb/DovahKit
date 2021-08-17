#include "wrap_native_object.h"
#include "../ui/generic/CanvasWidget.h"
#include "core/subsystems/coordinator.h"
#include "core/subsystems/userdata.h"
#include "core/subsystems/resources/DovahscriptResource.h"

#include "wrapper.h"

namespace dovahscript {
   [[nodiscard]] extern wrapper push_native_object(dovah::form_stub& stub) {
      wrapper out;
      out.stub = &stub;
      out.type = wrapper_type::form;
      return out;
   }

   [[nodiscard]] extern wrapper wrap_native_object(DovahscriptResource& resource) {
      wrapper out;
      out.managed_resource = &resource;
      out.type             = wrapper_type::lua_managed_resource;
      return out;
   }

   [[nodiscard]] extern wrapper wrap_native_object(DovahscriptResourceHandle resource) {
      assert(resource.bare() != nullptr);
      return wrap_native_object(resource.bare());
   }

   [[nodiscard]] extern wrapper wrap_native_object(QObject& object) {
      if (object.isWidgetType()) {
         wrapper out;
         out.widget = (QWidget*) &object;
         out.type   = wrapper_type::widget;
         return out;
      }
      if (auto* casted = qobject_cast<QButtonGroup*>(&object)) {
         wrapper out;
         out.button_group = casted;
         out.type         = wrapper_type::button_group;
         return out;
      }
      if (auto* casted = qobject_cast<CanvasWidgetEntity*>(&object)) {
         wrapper out;
         out.canvas_entity = casted;
         out.type          = wrapper_type::canvas_entity;
         return out;
      }
      if (auto* casted = qobject_cast<LuaScriptableCanvasWidgetLayerData*>(&object)) {
         wrapper out;
         out.canvas_layer_data = casted;
         out.type              = wrapper_type::canvas_layer_data;
         return out;
      }
      if (auto* casted = qobject_cast<DovahscriptResource*>(&object)) {
         return wrap_native_object(*casted);
      }
      assert(false && "unknown QObject type passed to push_native_object");
   }
}