#pragma once
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"
#include "editor/subsystems/worldedit/tool_system/tools/move_selection.h"
#include "ui_move_selection_tool_options.h"

class QComboBox;

namespace dovahkit::ui::worldedit::tools {
   class move_selection : public QWidget {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::move_selection;
         using options_type = tool::options;

      public:
         move_selection(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsMoveSelection ui;
         struct {
            options_type current_options = {};
         } _state;
         QButtonGroup* _typeButtonGroup = nullptr;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> transformFrame;
            EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> constraintFrame;
         } _widget_wrappers;
   };
}