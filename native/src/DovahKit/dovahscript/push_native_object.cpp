#include "push_native_object.h"
#include <array>
#include <QButtonGroup>
#include "../ui/generic/CanvasWidget.h"
#include "core/subsystems/coordinator.h"
#include "core/subsystems/userdata.h"
#include "core/subsystems/resources/DovahscriptResource.h"
#include "qt/DovahscriptCanvasWidgetLayerData.h"
#include "../dovah/form_stub.h"

#include "wrapper.h"
#include "wrappers/form/_all.h"
#include "wrappers/resource/_all.h"
#include "wrappers/ui/_all.h"
#include "wrappers/ini/setting.h"

namespace {
   std::array form_classes = {
      std::pair{ dovah::form_type::none,             dovahscript::wrappers::form::metatable_key },
      std::pair{ dovah::form_type::actor,            dovahscript::wrappers::actor::metatable_key },
      std::pair{ dovah::form_type::arrow,            dovahscript::wrappers::placed_arrow::metatable_key },
      std::pair{ dovah::form_type::barrier,          dovahscript::wrappers::placed_barrier::metatable_key },
      std::pair{ dovah::form_type::beam,             dovahscript::wrappers::placed_beam::metatable_key },
      std::pair{ dovah::form_type::cell,             dovahscript::wrappers::cell::metatable_key },
      std::pair{ dovah::form_type::cone,             dovahscript::wrappers::placed_cone::metatable_key },
      std::pair{ dovah::form_type::dialogue_branch,  dovahscript::wrappers::dialogue_branch::metatable_key },
      std::pair{ dovah::form_type::flame,            dovahscript::wrappers::placed_flame::metatable_key },
      std::pair{ dovah::form_type::formlist,         dovahscript::wrappers::formlist::metatable_key },
      std::pair{ dovah::form_type::grenade,          dovahscript::wrappers::placed_grenade::metatable_key },
      std::pair{ dovah::form_type::land,             dovahscript::wrappers::landscape::metatable_key },
      std::pair{ dovah::form_type::land_texture,     dovahscript::wrappers::land_texture::metatable_key },
      std::pair{ dovah::form_type::reference,        dovahscript::wrappers::objectreference::metatable_key },
      std::pair{ dovah::form_type::missile,          dovahscript::wrappers::placed_missile::metatable_key },
      std::pair{ dovah::form_type::navmesh,          dovahscript::wrappers::navmesh::metatable_key },
      std::pair{ dovah::form_type::navmesh_info_map, dovahscript::wrappers::navmesh_info_map::metatable_key },
      std::pair{ dovah::form_type::placed_hazard,    dovahscript::wrappers::placed_hazard::metatable_key },
      std::pair{ dovah::form_type::quest,            dovahscript::wrappers::quest::metatable_key },
      std::pair{ dovah::form_type::shout,            dovahscript::wrappers::shout::metatable_key },
      std::pair{ dovah::form_type::statik,           dovahscript::wrappers::statik::metatable_key },
      std::pair{ dovah::form_type::texture_set,      dovahscript::wrappers::texture_set::metatable_key },
      std::pair{ dovah::form_type::topic,            dovahscript::wrappers::topic::metatable_key },
      std::pair{ dovah::form_type::topic_info,       dovahscript::wrappers::topic_info::metatable_key },
      std::pair{ dovah::form_type::voicetype,        dovahscript::wrappers::voicetype::metatable_key },
      std::pair{ dovah::form_type::word_of_power,    dovahscript::wrappers::word_of_power::metatable_key },
      std::pair{ dovah::form_type::worldspace,       dovahscript::wrappers::worldspace::metatable_key },
   };

   // This should've been able to be a template, but Qt's janky build system causes the linker to puke when I do that
   #define QOBJECT_CLASS_PAIR(wrapper_name) std::pair{ &dovahscript::wrappers::ui::wrapper_name::wrapped_type::staticMetaObject, dovahscript::wrappers::ui::wrapper_name::metatable_key }
   //
   std::array qobject_classes = {
      QOBJECT_CLASS_PAIR(widget),
      //
      // Widget types:
      //
      QOBJECT_CLASS_PAIR(button),
      QOBJECT_CLASS_PAIR(canvas),
      QOBJECT_CLASS_PAIR(checkbox),
      QOBJECT_CLASS_PAIR(color_button),
      QOBJECT_CLASS_PAIR(dropdown),
      QOBJECT_CLASS_PAIR(file_save_button),
      QOBJECT_CLASS_PAIR(formpicker),
      QOBJECT_CLASS_PAIR(groupbox),
      QOBJECT_CLASS_PAIR(image_widget),
      QOBJECT_CLASS_PAIR(line),
      QOBJECT_CLASS_PAIR(progress_bar),
      QOBJECT_CLASS_PAIR(radio_button),
      QOBJECT_CLASS_PAIR(scrollbox),
      QOBJECT_CLASS_PAIR(spinbox),
      QOBJECT_CLASS_PAIR(tabbox),
      QOBJECT_CLASS_PAIR(tabbox_tab),
      QOBJECT_CLASS_PAIR(table_view),
      QOBJECT_CLASS_PAIR(text),
      QOBJECT_CLASS_PAIR(textarea),
      QOBJECT_CLASS_PAIR(textbox),
      QOBJECT_CLASS_PAIR(window),
      //
      // Other QObjects:
      //
      QOBJECT_CLASS_PAIR(radio_group),
      QOBJECT_CLASS_PAIR(canvas_layer),
      QOBJECT_CLASS_PAIR(canvas_layer_group),
      QOBJECT_CLASS_PAIR(canvas_text_data),
      std::pair{ &cobb::qt::ini::Setting::staticMetaObject, dovahscript::wrappers::ini::setting::metatable_key },
   };
   //
   #undef QOBJECT_CLASS_PAIR

   std::array resource_classes = {
      std::pair{ dovahscript::resource_type::undefined, (const char*)nullptr },
      std::pair{ dovahscript::resource_type::dds,       dovahscript::wrappers::resource::dds::metatable_key },
      std::pair{ dovahscript::resource_type::raster,    dovahscript::wrappers::resource::raster::metatable_key },
      std::pair{ dovahscript::resource_type::binary,    dovahscript::wrappers::resource::binary_view::metatable_key },
   };

   const char* get_metatable_for_qobject(const QMetaObject* rtti) {
      //
      // Use a do-while loop instead of mass qobject_casts both because I suspect it'd be 
      // faster, and to properly handle the possibility that a class and its subclass might 
      // both be in the list with different metatables (we want to ensure we'd pick the 
      // subclass).
      //
      do {
         for (auto& pair : qobject_classes)
            if (rtti == pair.first)
               return pair.second;
      } while (rtti = rtti->superClass());
      return nullptr;
   }
}

namespace dovahscript {
   extern int push_native_object(dovah::form_stub* stub) {
      auto* L = core::subsystems::coordinator::get().lua_state;
      if (!stub) {
         lua_pushnil(L);
         return 1;
      }
      const char* metatable = nullptr;
      for (auto& pair : form_classes) {
         if (pair.first == stub->form_type) {
            metatable = pair.second;
            break;
         }
      }
      if (!metatable) {
         if (dovah::form_type_is_reference(stub->form_type)) { // future-proofing: REFR subclasses fall back to being wrapped as if they were REFRs
            metatable = wrappers::objectreference::metatable_key;
         } else {
            metatable = wrappers::form::metatable_key;
         }
      }
      
      wrapper out;
      out.stub = stub;
      out.type = wrapper_type::form;
      return core::subsystems::userdata::get().push(L, out, metatable);
   }

   extern int push_native_object(const dovah::form_reference_t& ref) {
      return push_native_object(ref.get_form_stub());
   }

   extern int push_native_object(DovahscriptResource* resource) {
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

   extern int push_native_object(QObject* object) {
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
      // Now that we know what Lua class to use for this object, let's figure out what kind of 
      // pointer it has, and how to store it on a wrapper.
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
         assert(metatable);
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
      if (auto* casted = qobject_cast<DovahscriptCanvasWidgetLayerData*>(object)) {
         assert(metatable);
         wrapper out;
         out.canvas_layer_data = casted;
         out.type              = wrapper_type::canvas_layer_data;
         return core::subsystems::userdata::get().push(L, out, metatable);
      }
      if (auto* casted = qobject_cast<DovahscriptResource*>(object)) {
         return push_native_object(casted);
      }
      if (auto* casted = qobject_cast<cobb::qt::ini::Setting*>(object)) {
         assert(metatable);
         wrapper out;
         out.game_ini_setting = casted;
         out.type             = wrapper_type::ini_setting;
         return core::subsystems::userdata::get().push(L, out, metatable);
      }
      //
      // NOTE: It's not enough just to specify a metatable for a given QObject class; you also have 
      // to actually indicate what kind of "pertinent pointer," on the wrapper, it needs to be stored 
      // in, using the if-statements above.
      //
      assert(false && "unknown QObject type passed to push_native_object");
      //
      return 0;
   }
}