#pragma once
#include <QObject>
#include <QPointer>
#include <QWidget>
#include "./chrono.h"
#include "./bind_tree/tree.h"
#include "./devices/abstract_device_handler.h"
#include "./devices/keyboard_mouse.h"
#include "./devices/xinput.h"
#include "./enums/input_device_type.h"

namespace dovahkit::subsystems::worldinput2 {
   class combined_tool_results;
}

namespace dovahkit::subsystems::worldinput2 {
   class core : public QObject {
      Q_OBJECT;
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

         void viewFocusChange(QWidget* target, bool has_focus);
         void update(double& elapsed_time);

      public slots:
         void setTargetWidget(QWidget* target);
         void setBindingsFor(input_device_type, const binds::tree&);

      protected slots:
         void ignoreAllHeldKeys();
         void doPerFrameInputProcessing(double& elapsed_seconds, combined_tool_results& out); // both args are out-variables

      protected:
         struct {
            devices::keyboard_mouse keyboard_mouse;
            devices::xinput         gamepad;
         } device_handlers;
         struct {
            QPointer<QWidget> target_widget;
            timestamp_t last_update = zero_timestamp;
         } state;
         struct {
            binds::tree keyboard = binds::tree(input_device_type::keyboard_mouse);
            binds::tree gamepad  = binds::tree(input_device_type::xinput);
         } binds;
   };
}
