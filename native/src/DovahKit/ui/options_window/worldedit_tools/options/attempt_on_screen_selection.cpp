#include "./attempt_on_screen_selection.h"
#include <array>
#include <type_traits>

namespace dovahkit::ui::worldedit::tools {
   attempt_on_screen_selection::attempt_on_screen_selection(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);

      this->_widget_wrappers.operation = this->ui.operation;
      {
         auto& widget = this->_widget_wrappers.operation;

         using enum_type = std::decay_t<decltype(widget)>::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::no_op,   "Do nothing" },
            item_type{ enum_type::add,     "Add to selection" },
            item_type{ enum_type::remove,  "Remove from selection" },
            item_type{ enum_type::toggle,  "Toggle selected" },
            item_type{ enum_type::replace, "Replace selection" },
         };

         widget.addItems(items);
         widget.beginOneWaySync(this->_state.current_options.operation);
      }
   }

   void attempt_on_screen_selection::set_options(const options_type& v) {
      this->_state.current_options = v;

      this->_widget_wrappers.operation.setValueSilent(v.operation);
   }
}