#include "./turn_camera.h"
#include <array>
#include <type_traits>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   turn_camera::turn_camera(QWidget* parent) : QWidget(parent) {
      auto* layout = new QGridLayout(this);
      this->setLayout(layout);

      int row = 0;

      {
         layout->addWidget(new QLabel(tr("Magnitude:")), row, 0);

         auto* wrapper = new QWidget;
         layout->addWidget(wrapper, row, 1);

         auto* wr_layout = new QHBoxLayout;
         wrapper->setLayout(wr_layout);

         this->_subwidgets.magnitude.yaw   = new QDoubleSpinBox;
         this->_subwidgets.magnitude.pitch = new QDoubleSpinBox;

         wr_layout->addWidget(new QLabel(tr("Yaw", "axis label")));
         wr_layout->addWidget(this->_subwidgets.magnitude.yaw);
         wr_layout->addWidget(new QLabel(tr("Pitch", "axis label")));
         wr_layout->addWidget(this->_subwidgets.magnitude.pitch);
      }
      ++row;

      {
         auto* groupbox = new QGroupBox(tr("Range control mapping"), this);
         layout->addWidget(groupbox, row, 0);

         int   gb_row = 0;
         auto* gb_layout = new QGridLayout(groupbox);
         groupbox->setLayout(gb_layout);

         {
            auto* explain = new QLabel(tr("When this tool is bound to a range control, such as a joystick, the control's position will be multiplied into the tool's magnitude. You can control how the control is mapped here.", "range control explanation"));
            explain->setWordWrap(true);
            gb_layout->addWidget(explain, gb_row, 0, 1, 2);
            ++gb_row;
         }

         this->_subwidgets.range.x.axis = new QComboBox;
         this->_subwidgets.range.x.sign = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("Input X-Axis:")), gb_row, 0);
         gb_layout->addWidget(this->_subwidgets.range.x.axis, gb_row, 1);
         gb_layout->addWidget(this->_subwidgets.range.x.sign, gb_row, 2);
         ++gb_row;

         this->_subwidgets.range.y.axis = new QComboBox;
         this->_subwidgets.range.y.sign = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("Input Y-Axis:")), gb_row, 0);
         gb_layout->addWidget(this->_subwidgets.range.y.axis, gb_row, 1);
         gb_layout->addWidget(this->_subwidgets.range.y.sign, gb_row, 2);
         ++gb_row;
      }
      ++row;

      //
      // Layout done; set up the fields:
      //

      {
         auto& widgets = this->_subwidgets.magnitude;
         widgets.yaw->setRange(-360, 360);
         widgets.pitch->setRange(-360, 360);
         
         auto& fields = this->_state.current_options.magnitudes;
         cobb::qt::bind(widgets.yaw,   fields.yaw);
         cobb::qt::bind(widgets.pitch, fields.pitch);
      }
      {
         auto& widgets = this->_subwidgets.range;
         auto& fields  = this->_state.current_options.range;

         {
            using enum_type = decltype(widgets.x.axis)::value_type;
            using item_type = std::pair<enum_type, const char*>;

            constexpr const auto items = std::array{
               item_type{ enum_type::yaw,   "Yaw" },
               item_type{ enum_type::pitch, "Pitch" },
            };

            widgets.x.axis.addItems(items);
            widgets.y.axis.addItems(items);
            //
            widgets.x.axis.beginOneWaySync(fields.x.axis);
            widgets.y.axis.beginOneWaySync(fields.y.axis);
         }
         {
            using enum_type = decltype(widgets.x.sign)::value_type;
            using item_type = std::pair<enum_type, const char*>;

            constexpr const auto items = std::array{
               item_type{ enum_type::positive, "Positive" },
               item_type{ enum_type::negative, "Negative" },
            };

            widgets.x.sign.addItems(items);
            widgets.y.sign.addItems(items);
            //
            widgets.x.sign.beginOneWaySync(fields.x.sign);
            widgets.y.sign.beginOneWaySync(fields.y.sign);
         }
      }
   }

   void turn_camera::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         auto& widgets = this->_subwidgets.magnitude;

         const auto blocker_z = QSignalBlocker(widgets.yaw);
         const auto blocker_x = QSignalBlocker(widgets.pitch);

         widgets.yaw->setValue(v.magnitudes.yaw);
         widgets.pitch->setValue(v.magnitudes.pitch);
      }
      {
         const auto& fields = this->_state.current_options.range;
         auto& widgets = this->_subwidgets.range;

         widgets.x.axis.setValueSilent(fields.x.axis);
         widgets.y.axis.setValueSilent(fields.y.axis);

         widgets.x.sign.setValueSilent(fields.x.sign);
         widgets.y.sign.setValueSilent(fields.y.sign);
      }
   }
}