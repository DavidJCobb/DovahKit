#pragma once
#include <QWidget>
#include "editor/subsystems/worldinput/enums/range_input_axes.h"
#include "editor/subsystems/worldinput/enums/range_input_control.h"

namespace dovahkit::ui::worldedit::tools {
   class base : public QWidget {
      public:
         using QWidget::QWidget;

         virtual void onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control, dovahkit::subsystems::worldinput::range_input_axes) {}
   };
}