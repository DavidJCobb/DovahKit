#pragma once
#include <QObject>
#include <QPointer>
#include <QWidget>
#include "./devices/abstract_device_handler.h"
#include "./devices/keyboard_mouse.h"
#include "./devices/xinput.h"
#include "./enums/input_device_type.h"
#include "./bind_list.h"
#include "./chrono.h"
#include "./raycast_result.h"

namespace dovahkit::subsystems::worldedit {
   class tool_response_tuple;
}
namespace dovahkit::subsystems::worldinput {
   class control_scheme;
}

namespace dovahkit::subsystems::worldinput {
   class core : public QObject {
      Q_OBJECT;
      public:
         using tool_response_tuple = worldedit::tool_response_tuple;

      protected:
         core();
         ~core();
      public:
         static core& get() {
            static core instance;
            return instance;
         }

      public:
         const devices::abstract_device_handler& device_by_type(input_device_type) const;
         devices::abstract_device_handler& device_by_type(input_device_type);

      public slots:
         void setTargetWidget(QWidget* target);
         void setBindingsFor(const control_scheme&);

      protected slots:
         void ignoreAllHeldKeys();
      public slots:
         void doPerFrameInputProcessing(double& elapsed_seconds, tool_response_tuple& out); // both args are out-variables

      protected:
         struct {
            devices::keyboard_mouse keyboard_mouse;
            devices::xinput         gamepad;
         } device_handlers;
         struct {
            QPointer<QWidget> target_widget;
            timestamp_t last_update = zero_timestamp;
            //
            bool target_widget_has_focus = false;
         } state;
         struct {
            bind_list keyboard = bind_list(input_device_type::keyboard_mouse);
            bind_list gamepad  = bind_list(input_device_type::xinput);
         } binds;

      protected:

         // Detects focus gain and loss on the target widget
         virtual bool eventFilter(QObject* watched, QEvent* event);
   };
}
