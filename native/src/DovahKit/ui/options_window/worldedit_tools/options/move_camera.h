#pragma once
#include <QSpinBox>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"
#include "editor/subsystems/worldedit/tool_system/tools/move_camera.h"
#include "./util/WorldeditToolRangeInputScalesWidget.h"

class QComboBox;

namespace dovahkit::ui::worldedit::tools {
   class move_camera : public QWidget {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::move_camera;
         using options_type = tool::options;

      public:
         move_camera(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         struct {
            options_type current_options = {};
         } _state;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> reference_frame;
            struct {
               QDoubleSpinBox* x = nullptr;
               QDoubleSpinBox* y = nullptr;
               QDoubleSpinBox* z = nullptr;
            } magnitude;
            WorldeditToolRangeInputScalesWidget* range = nullptr;
         } _subwidgets;
   };
}