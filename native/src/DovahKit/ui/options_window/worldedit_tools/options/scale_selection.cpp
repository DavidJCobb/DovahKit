#include "./scale_selection.h"
#include <array>
#include <type_traits>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   scale_selection::scale_selection(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);
      {
         auto& wx = this->_widget_wrappers.rangeXSign;
         auto& wy = this->_widget_wrappers.rangeYSign;
         wx = this->ui.rangeXSign;
         wy = this->ui.rangeYSign;

         using enum_type = dovahkit::subsystems::worldedit::sign;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::positive, "Positive" },
            item_type{ enum_type::negative, "Negative" },
         };

         wx.addItems(items);
         wy.addItems(items);

         wx.addChangeListener([this](enum_type v) {
            auto& data = this->_state.current_options;
            if (!data.range.has_value())
               return;
            data.range.value().x = v;
         });
         wy.addChangeListener([this](enum_type v) {
            auto& data = this->_state.current_options;
            if (!data.range.has_value())
               return;
            data.range.value().y = v;
         });
      }
      this->ui.magnitude->setRange(-500, 500);

      cobb::qt::bind(this->ui.magnitude,        this->_state.current_options.mod);
      cobb::qt::bind(this->ui.scaleAllTogether, this->_state.current_options.scale_all_together);
      QObject::connect(this->ui.rangeGroupbox, &QGroupBox::toggled, this, [this](bool checked) {
         auto& data = this->_state.current_options;
         if (checked) {
            if (data.range.has_value())
               return;
            auto& range = data.range.emplace();
            range.x = this->_widget_wrappers.rangeXSign.value();
            range.y = this->_widget_wrappers.rangeYSign.value();
         } else {
            data.range = {};
         }
      });
   }

   void scale_selection::set_options(const options_type& v) {
      this->_state.current_options = v;

      const auto blocker_0 = QSignalBlocker(this->ui.magnitude);
      const auto blocker_1 = QSignalBlocker(this->ui.scaleAllTogether);
      const auto blocker_2 = QSignalBlocker(this->ui.rangeGroupbox);

      this->ui.magnitude->setValue(v.mod);
      this->ui.scaleAllTogether->setChecked(v.scale_all_together);

      this->ui.rangeGroupbox->setChecked(v.range.has_value());
      {
         auto& wx = this->_widget_wrappers.rangeXSign;
         auto& wy = this->_widget_wrappers.rangeYSign;
         if (v.range.has_value()) {
            const auto& range = v.range.value();
            wx.setValueSilent(range.x);
            wy.setValueSilent(range.y);
         } else {
            wx.setValueSilent(dovahkit::subsystems::worldedit::sign::positive);
            wy.setValueSilent(dovahkit::subsystems::worldedit::sign::positive);
         }
      }
   }

   void scale_selection::onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control ctrl, dovahkit::subsystems::worldinput::range_input_axes axes) {
      using namespace dovahkit::subsystems::worldinput;

      bool any_control = ctrl != range_input_control::none;
      this->ui.rangeGroupbox->setEnabled(any_control);

      bool control_has_two_axes = !any_control || range_input_control_has_multiple_axes(ctrl);
      if (!control_has_two_axes) {
         axes = range_input_axes::x;
      }

      bool x_visible = true;
      bool y_visible = true;
      if (axes != range_input_axes::all) {
         x_visible = (axes == range_input_axes::x);
         y_visible = !x_visible;
      }

      this->ui.rangeXLabel->setVisible(x_visible);
      this->ui.rangeXSign->setVisible(x_visible);

      this->ui.rangeYLabel->setVisible(y_visible);
      this->ui.rangeYSign->setVisible(y_visible);

      if (!control_has_two_axes) {
         this->ui.rangeXLabel->setText(tr("Input Axis:"));
      } else {
         this->ui.rangeXLabel->setText(tr("Input X-Axis:"));
      }
   }
}