#include "./modify_camera_speed_flags.h"
#include <array>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

namespace dovahkit::ui::worldedit::tools {
   modify_camera_speed_flags::modify_camera_speed_flags(QWidget* parent) : QWidget(parent) {
      auto* layout = new QGridLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      this->setLayout(layout);

      {
         auto* label = new QLabel(tr("Boost:"), this);
         layout->addWidget(label, 0, 0);
      }
      this->_subwidgets.boost = new QComboBox(this);
      layout->addWidget(this->_subwidgets.boost, 0, 1);

      {
         auto* label = new QLabel(tr("Precision:"), this);
         layout->addWidget(label, 1, 0);
      }
      this->_subwidgets.precision = new QComboBox(this);
      layout->addWidget(this->_subwidgets.precision, 1, 1);

      {
         auto& widgets = this->_subwidgets;

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

      using enum_type = decltype(this->_subwidgets.boost)::value_type;
      constexpr const auto fallback = enum_type::no_op;

      if (!this->_subwidgets.boost.setValueSilent(v.boost, fallback))
         this->_state.current_options.boost = fallback;
      if (!this->_subwidgets.precision.setValueSilent(v.precision, fallback))
         this->_state.current_options.precision = fallback;
   }
}