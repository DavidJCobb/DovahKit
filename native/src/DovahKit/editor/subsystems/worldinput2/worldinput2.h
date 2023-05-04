#pragma once
#include <QObject>
#include <QPointer>
#include <QWidget>
#include "./bind_list.h"
#include "./chrono.h"
#include "./bind_tree/tree.h"
#include "./devices/abstract_device_handler.h"
#include "./devices/keyboard_mouse.h"
#include "./devices/xinput.h"
#include "./enums/input_device_type.h"

namespace dovahkit::subsystems::worldedit {
   class tool_results_tuple;
}
namespace dovahkit::subsystems::worldinput2 {
   class combined_tool_results;
}

namespace dovahkit::subsystems::worldinput2 {
   class core : public QObject {
      Q_OBJECT;
      public:
         using tool_results_tuple = worldedit::tool_results_tuple;

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
         void setBindingsFor(input_device_type, const binds::tree&);

      protected slots:
         void ignoreAllHeldKeys();
      public slots:
         void doPerFrameInputProcessing(double& elapsed_seconds, tool_results_tuple& out); // both args are out-variables

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
         struct {
            binds::tree keyboard = binds::tree(input_device_type::keyboard_mouse);
            binds::tree gamepad  = binds::tree(input_device_type::xinput);
         } schemes;

      protected:

         // Detects focus gain and loss on the target widget
         virtual bool eventFilter(QObject* watched, QEvent* event);
   };
}
