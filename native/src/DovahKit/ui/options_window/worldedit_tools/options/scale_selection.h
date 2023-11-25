#pragma once
#include <QButtonGroup>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/scale_selection.h"
#include "ui_scale_selection_tool_options.h" // generated

namespace dovahkit::ui::worldedit::tools {
   class scale_selection : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::scale_selection;
         using options_type = tool::options;

      public:
         scale_selection(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

         virtual void onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control, dovahkit::subsystems::worldinput::range_input_axes) override;

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsScaleSelection ui;
         struct {
            options_type current_options = {};
         } _state;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::sign> rangeXSign;
            EnumCombobox<dovahkit::subsystems::worldedit::sign> rangeYSign;
         } _widget_wrappers;
   };
}