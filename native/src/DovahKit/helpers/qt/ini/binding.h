#pragma once
#include "../ini.h"

namespace cobb::qt::ini {

   /*
   //
   // Syntax for these helpers:
   //
   //    Setting*   setting      = ...;
   //    QCheckBox* checkbox_set = ...;
   //
   //    cobb::qt::ini::bindSettingControl(*setting, checkbox_set, &QCheckBox::setChecked, &QCheckBox::toggled);
   //
   */


   // Helper for setting up a UI control so that it can be used to change the pending value of a setting, 
   // as in an options window. This variant is for controls whose "changed" signal does not pass the new 
   // control state as an argument.
   template<typename widget_type, typename Wx, typename value_type> requires (std::is_base_of_v<Wx, widget_type>)
   void bindSettingControl(
      Setting& setting,
      widget_type* widget,
      void (Wx::*setter)(value_type),
      value_type(Wx::*getter)(),
      void (Wx::*signal)()
   ) {
      auto data = setting.pendingValue();
      if (!data.isValid())
         data = setting.currentValue();
      (widget->*setter)(data.value<std::decay_t<value_type>>());
      //
      QObject::connect(&setting, &Setting::pendingValueDiscarded, widget, [widget, setter, &setting]() {
         const auto blocker = QSignalBlocker(widget);
         (widget->*setter)(setting.currentValue().value<std::decay_t<value_type>>());
      }, Qt::ConnectionType::QueuedConnection);
      //
      QObject::connect(&widget, signal, &setting, [widget, &setting]() {
         setting.setPendingValue((widget->*getter)());
      });
   };

   // Helper for setting up a UI control so that it can be used to change the pending value of a setting, 
   // as in an options window. This variant is for controls whose "changed" signal passes the new control 
   // state as an argument.
   template<typename widget_type, typename Wx, typename value_type> requires (std::is_base_of_v<Wx, widget_type>)
   void bindSettingControl(
      Setting& setting,
      widget_type* widget,
      void (Wx::*setter)(value_type),
      void (Wx::*signal)(value_type)
   ) {
      auto data = setting.pendingValue();
      if (!data.isValid())
         data = setting.currentValue();
      (widget->*setter)(data.value<std::decay_t<value_type>>());
      //
      QObject::connect(&setting, &Setting::pendingValueDiscarded, widget, [widget, setter, &setting]() {
         const auto blocker = QSignalBlocker(widget);
         (widget->*setter)(setting.currentValue().value<std::decay_t<value_type>>());
      }, Qt::ConnectionType::QueuedConnection);
      //
      QObject::connect(widget, signal, &setting, [&setting](value_type v) {
         setting.setPendingValue(v);
      });
   };
}