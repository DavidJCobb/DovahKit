#include "widget_overrides.h"

namespace dovahscript {
   [[nodiscard]] extern QWidget* get_widget_forced_parent(QWidget* widget) {
      auto data = widget->property("dovahscript: widget forced parent");
      if (data.isValid())
         return (QWidget*)data.value<QObject*>();
      return nullptr;
   }
   extern void set_widget_forced_parent(QWidget* subject, QWidget* parent) {
      subject->setProperty("dovahscript: widget forced parent", QVariant::fromValue<QObject*>(parent));
   }

   [[nodiscard]] extern const char* get_widget_metatable_override(QWidget* widget) {
      auto data = widget->property("dovahscript: force widget metatable key to");
      if (data.isValid())
         return (const char*)data.value<void*>();
      return nullptr;
   }
   extern void set_widget_metatable_override(QWidget* widget, const char* metatable_key) {
      assert(widget && metatable_key);
      widget->setProperty("dovahscript: force widget metatable key to", QVariant::fromValue<void*>((void*)metatable_key));
   }
}