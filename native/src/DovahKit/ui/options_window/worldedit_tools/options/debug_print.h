#pragma once
#include <QWidget>
#include "editor/subsystems/worldedit/tool_system/tools/debug_print.h"

class QLineEdit;

namespace dovahkit::ui::worldedit::tools {
   class debug_print : public QWidget {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::debug_print;
         using options_type = tool::options;

      public:
         debug_print(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         struct {
            options_type current_options = {};
         } _state;
         struct {
            QLineEdit* text = nullptr;
         } _subwidgets;
   };
}