#include "widget_overrides.h"
#include "core/verify_threading.h"

namespace {
   static constexpr const char* property_for_parent_override = "dovahscript: widget forced parent";
}

namespace dovahscript {
   [[nodiscard]] extern QWidget* get_widget_forced_parent(QWidget* widget) {
      core::require_client_thread();
      //
      auto data = widget->property(property_for_parent_override);
      if (data.isValid())
         return (QWidget*)data.value<QObject*>();
      return nullptr;
   }
   extern void set_widget_forced_parent(QWidget* subject, QWidget* parent) {
      core::require_client_thread();
      //
      subject->setProperty(property_for_parent_override, QVariant::fromValue<QObject*>(parent));
   }
}