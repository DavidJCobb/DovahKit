#pragma once
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/modify_camera_speed_flags.h"
#include "ui_modify_camera_speed_flags_tool_options.h" // generated

class QComboBox;

namespace dovahkit::ui::worldedit::tools {
   class modify_camera_speed_flags : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::modify_camera_speed_flags;
         using options_type = tool::options;

      public:
         modify_camera_speed_flags(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsModifyCameraSpeedFlags ui;
         struct {
            options_type current_options = {};
         } _state;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::bool_operation> boost;
            EnumCombobox<dovahkit::subsystems::worldedit::bool_operation> precision;
         } _widget_wrappers;
   };
}