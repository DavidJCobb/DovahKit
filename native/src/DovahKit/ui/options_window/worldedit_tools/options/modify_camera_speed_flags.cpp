#include "./modify_camera_speed_flags.h"
#include <array>

namespace dovahkit::ui::worldedit::tools {
   modify_camera_speed_flags::modify_camera_speed_flags(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);

      this->_widget_wrappers.boost     = this->ui.boostOp;
      this->_widget_wrappers.precision = this->ui.precisionOp;

      {
         auto& widgets = this->_widget_wrappers;

         using enum_type = decltype(widgets.boost)::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::no_op,     "Do nothing" },
            item_type{ enum_type::set_true,  "Enable" },
            item_type{ enum_type::set_false, "Disable" },
            item_type{ enum_type::invert,    "Toggle" },
         };
         widgets.boost.addItems(items);
         widgets.precision.addItems(items);

         widgets.boost.beginOneWaySync(this->_state.current_options.boost);
         widgets.precision.beginOneWaySync(this->_state.current_options.precision);
      }
   }

   void modify_camera_speed_flags::set_options(const options_type& v) {
      this->_state.current_options = v;

      using enum_type = decltype(this->_widget_wrappers.boost)::value_type;
      constexpr const auto fallback = enum_type::no_op;

      if (!this->_widget_wrappers.boost.setValueSilent(v.boost, fallback))
         this->_state.current_options.boost = fallback;
      if (!this->_widget_wrappers.precision.setValueSilent(v.precision, fallback))
         this->_state.current_options.precision = fallback;
   }
}