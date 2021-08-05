#include "push_native_object.h"
#include <array>
#include "../ui/generic/CanvasWidget.h"
#include "core/subsystems/coordinator.h"
#include "core/subsystems/userdata.h"
#include "core/subsystems/resources/DovahscriptResource.h"
#include "../dovah/form_stub.h"

#include "wrapper.h"
#include "wrappers/form/_all.h"
#include "wrappers/resource/_all.h"
#include "wrappers/ui/_all.h"

namespace {
   std::array form_classes = {
      std::pair{ dovah::form_type::none,          dovahscript::wrappers::form::metatable_key },
      /*//
      std::pair{ dovah::form_type::cell,          dovahscript::wrappers::cell::metatable_key },
      std::pair{ dovah::form_type::formlist,      dovahscript::wrappers::formlist::metatable_key },
      std::pair{ dovah::form_type::land,          dovahscript::wrappers::landscape::metatable_key },
      std::pair{ dovah::form_type::land_texture,  dovahscript::wrappers::land_texture::metatable_key },
      std::pair{ dovah::form_type::quest,         dovahscript::wrappers::quest::metatable_key },
      std::pair{ dovah::form_type::shout,         dovahscript::wrappers::shout::metatable_key },
      std::pair{ dovah::form_type::texture_set,   dovahscript::wrappers::texture_set::metatable_key },
      std::pair{ dovah::form_type::topic,         dovahscript::wrappers::topic::metatable_key },
      std::pair{ dovah::form_type::topic_info,    dovahscript::wrappers::topic_info::metatable_key },
      std::pair{ dovah::form_type::voicetype,     dovahscript::wrappers::voicetype::metatable_key },
      std::pair{ dovah::form_type::word_of_power, dovahscript::wrappers::word_of_power::metatable_key },
      std::pair{ dovah::form_type::worldspace,    dovahscript::wrappers::worldspace::metatable_key },
      //*/
   };

   std::array qobject_classes = {
      std::pair{ &QWidget::staticMetaObject, dovahscript::wrappers::ui::widget::metatable_key },
      /*//
      std::pair{ &QButtonGroup::staticMetaObject, dovahscript::wrappers::ui::radio_group::metatable_key },
      //
      // Widgets:
      //
      std::pair{ &QCheckBox::staticMetaObject,      dovahscript::wrappers::ui::checkbox::metatable_key },
      std::pair{ &QComboBox::staticMetaObject,      dovahscript::wrappers::ui::dropdown::metatable_key },
      std::pair{ &QDialog::staticMetaObject,        dovahscript::wrappers::ui::window::metatable_key },
      std::pair{ &QDoubleSpinBox::staticMetaObject, dovahscript::wrappers::ui::spinbox::metatable_key },
      std::pair{ &QGroupBox::staticMetaObject,      dovahscript::wrappers::ui::groupbox::metatable_key },
      std::pair{ &QLabel::staticMetaObject,         dovahscript::wrappers::ui::text::metatable_key },
      std::pair{ &QLineEdit::staticMetaObject,      dovahscript::wrappers::ui::textbox::metatable_key },
      std::pair{ &QProgressBar::staticMetaObject,   dovahscript::wrappers::ui::progress_bar::metatable_key },
      std::pair{ &QPushButton::staticMetaObject,    dovahscript::wrappers::ui::button::metatable_key },
      std::pair{ &QRadioButton::staticMetaObject,   dovahscript::wrappers::ui::radio_button::metatable_key },
      std::pair{ &QScrollArea::staticMetaObject,    dovahscript::wrappers::ui::scrollbox::metatable_key },
      std::pair{ &QTableView::staticMetaObject,     dovahscript::wrappers::ui::table_view::metatable_key },
      std::pair{ &QTabWidget::staticMetaObject,     dovahscript::wrappers::ui::tabbox::metatable_key },
      std::pair{ &CanvasWidget::staticMetaObject,   dovahscript::wrappers::ui::canvas::metatable_key },
      std::pair{ &FormPicker::staticMetaObject,     dovahscript::wrappers::ui::formpicker::metatable_key },
      std::pair{ &LuaManagedRasterWidget::staticMetaObject, dovahscript::wrappers::ui::image_widget::metatable_key },
      //
      // Miscellaneous:
      //
      std::pair{ &CanvasWidgetLayer::staticMetaObject,      dovahscript::wrappers::ui::canvas_layer::metatable_key },
      std::pair{ &CanvasWidgetLayerGroup::staticMetaObject, dovahscript::wrappers::ui::canvas_layer_group::metatable_key },
      //*/
   };

   std::array resource_classes = {
      std::pair{ dovahscript::resource_type::undefined, (const char*)nullptr },
      /*//
      std::pair{ dovahscript::resource_type::dds,       dovahscript::wrappers::resource::dds::metatable_key },
      std::pair{ dovahscript::resource_type::raster,    dovahscript::wrappers::resource::raster::metatable_key },
      std::pair{ dovahscript::resource_type::binary,    dovahscript::wrappers::resource::unknown::metatable_key },
      //*/
   };

   const char* get_metatable_for_qobject(const QMetaObject* rtti) {
      do {
         for (auto& pair : qobject_classes)
            if (rtti == pair.first)
               return pair.second;
      } while (rtti = rtti->superClass());
      return nullptr;
   }
}

namespace dovahscript {
   int push_native_object(dovah::form_stub* stub) {
      auto* L = core::subsystems::coordinator::get().lua_state;
      if (!stub) {
         lua_pushnil(L);
         return 1;
      }
      const char* metatable = wrappers::form::metatable_key;
      for (auto& pair : form_classes) {
         if (pair.first == stub->formType) {
            metatable = pair.second;
            break;
         }
      }
      //
      wrapper out;
      out.stub = stub;
      out.type = wrapper_type::form;
      return core::subsystems::userdata::get().push(L, out, metatable);
   }

   int push_native_object(DovahscriptResource* resource) {
      auto* L = core::subsystems::coordinator::get().lua_state;
      if (!resource) {
         lua_pushnil(L);
         return 1;
      }
      const char* metatable = nullptr;
      for (auto& pair : resource_classes) {
         if (resource->resource_type() == pair.first) {
            metatable = pair.second;
            break;
         }
      }
      assert(metatable);
      //
      wrapper out;
      out.managed_resource = resource;
      out.type             = wrapper_type::lua_managed_resource;
      return core::subsystems::userdata::get().push(L, out, metatable);
   }

   int push_native_object(DovahscriptResourceHandle resource) {
      return push_native_object(resource.bare());
   }

   int push_native_object(QObject* object) {
      auto* L = core::subsystems::coordinator::get().lua_state;
      if (!object) {
         lua_pushnil(L);
         return 1;
      }
      const char* metatable = nullptr;
      if (object->isWidgetType()) {
         metatable = wrappers::ui::widget::metatable_key; // default
      }
      if (auto* desired = get_metatable_for_qobject(object->metaObject()))
         metatable = desired;
      //
      if (object->isWidgetType()) {
         if (!metatable)
            metatable = wrappers::ui::widget::metatable_key;
         wrapper out;
         out.widget = (QWidget*)object;
         out.type   = wrapper_type::widget;
         return core::subsystems::userdata::get().push(L, out, metatable);
      }
      if (auto* casted = qobject_cast<QButtonGroup*>(object)) {
         /*//
         if (!metatable)
            metatable = wrappers::ui::radio_group::metatable_key;
         //*/
         wrapper out;
         out.button_group = casted;
         out.type         = wrapper_type::button_group;
         return core::subsystems::userdata::get().push(L, out, metatable);
      }
      if (auto* casted = qobject_cast<CanvasWidgetEntity*>(object)) {
         assert(metatable);
         wrapper out;
         out.canvas_entity = casted;
         out.type          = wrapper_type::canvas_entity;
         return core::subsystems::userdata::get().push(L, out, metatable);
      }
      if (auto* casted = qobject_cast<LuaScriptableCanvasWidgetLayerData*>(object)) {
         assert(metatable);
         wrapper out;
         out.canvas_layer_data = casted;
         out.type              = wrapper_type::canvas_layer_data;
         return core::subsystems::userdata::get().push(L, out, metatable);
      }
      if (auto* casted = qobject_cast<DovahscriptResource*>(object)) {
         return push_native_object(casted);
      }
      assert(false && "unknown QObject type passed to push_native_object");
   }
}