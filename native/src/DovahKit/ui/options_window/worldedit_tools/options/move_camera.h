#pragma once
#include <QSpinBox>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/move_camera.h"
#include "ui_move_camera_tool_options.h" // generated

namespace dovahkit::ui::worldedit::tools {
   class move_camera : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::move_camera;
         using options_type = tool::options;

      public:
         move_camera(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

         virtual void onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control, dovahkit::subsystems::worldinput::range_input_axes) override;

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsMoveCamera ui;
         struct {
            options_type current_options = {};
         } _state;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> transformFrame;
         } _widget_wrappers;
   };
}