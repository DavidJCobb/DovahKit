#pragma once
#include <QSpinBox>
#include <QWidget>

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/turn_camera.h"
#include "ui_turn_camera_tool_options.h" // generated

namespace dovahkit::ui::worldedit::tools {
   class turn_camera : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::turn_camera;
         using options_type = tool::options;

      public:
         turn_camera(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

         virtual void onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control, dovahkit::subsystems::worldinput::range_input_axes) override;

      protected:
         Ui::WorldeditToolOptionsTurnCamera ui;
         struct {
            options_type current_options = {};
         } _state;
   };
}