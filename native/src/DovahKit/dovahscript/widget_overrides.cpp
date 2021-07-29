#include "widget_overrides.h"

namespace {
   static constexpr const char* property_for_parent_override    = "dovahscript: widget forced parent";
   static constexpr const char* property_for_metatable_override = "dovahscript: force widget metatable key to";
}

namespace dovahscript {
   [[nodiscard]] extern QWidget* get_widget_forced_parent(QWidget* widget) {
      auto data = widget->property(property_for_parent_override);
      if (data.isValid())
         return (QWidget*)data.value<QObject*>();
      return nullptr;
   }
   extern void set_widget_forced_parent(QWidget* subject, QWidget* parent) {
      subject->setProperty(property_for_parent_override, QVariant::fromValue<QObject*>(parent));
   }

   [[nodiscard]] extern const char* get_widget_metatable_override(QWidget* widget) {
      auto data = widget->property(property_for_metatable_override);
      if (data.isValid())
         return (const char*)data.value<void*>();
      return nullptr;
   }
   extern void set_widget_metatable_override(QWidget* widget, const char* metatable_key) {
      assert(widget && metatable_key);
      widget->setProperty(property_for_metatable_override, QVariant::fromValue<void*>((void*)metatable_key));
   }
}