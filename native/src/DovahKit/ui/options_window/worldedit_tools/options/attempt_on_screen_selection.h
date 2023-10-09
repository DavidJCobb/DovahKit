#pragma once
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/attempt_on_screen_selection.h"
#include "ui_attempt_on_screen_selection_tool_options.h" // generated

namespace dovahkit::ui::worldedit::tools {
   class attempt_on_screen_selection : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::attempt_on_screen_selection;
         using options_type = tool::options;

      public:
         attempt_on_screen_selection(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsAttemptOnScreenSelection ui;
         struct {
            options_type current_options = {};
         } _state;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::selection_operation> operation;
         } _widget_wrappers;
   };
}