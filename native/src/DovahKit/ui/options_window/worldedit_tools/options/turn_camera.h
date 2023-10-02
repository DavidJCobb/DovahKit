#pragma once
#include <QSpinBox>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"
#include "editor/subsystems/worldedit/tool_system/tools/turn_camera.h"
#include "./util/WorldeditToolRangeInputScalesWidget.h"

class QComboBox;

namespace dovahkit::ui::worldedit::tools {
   class turn_camera : public QWidget {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::turn_camera;
         using options_type = tool::options;

      public:
         turn_camera(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         struct {
            options_type current_options = {};
         } _state;
         struct {
            struct {
               QDoubleSpinBox* yaw   = nullptr;
               QDoubleSpinBox* pitch = nullptr;
            } magnitude;
            WorldeditToolRangeInputScalesWidget* range = nullptr;
         } _subwidgets;
   };
}