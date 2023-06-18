#pragma once
#include <QWidget>
#include "editor/subsystems/worldedit/tool_system/tools/attempt_on_screen_selection.h"

class QComboBox;

namespace dovahkit::ui::worldedit::tools {
   class attempt_on_screen_selection : public QWidget {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::attempt_on_screen_selection;
         using options_type = tool::options;

      public:
         attempt_on_screen_selection(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         struct {
            options_type current_options = {};
         } _state;
         struct {
            QComboBox* operation = nullptr;
         } _subwidgets;
   };
}