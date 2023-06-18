#include "./move_camera.h"
#include <array>
#include <type_traits>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   move_camera::move_camera(QWidget* parent) : QWidget(parent) {
      auto* layout = new QGridLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      this->setLayout(layout);

      int row = 0;

      {
         auto* groupbox = new QGroupBox(tr("Reference frames"), this);
         layout->addWidget(groupbox, row, 0, 1, 2);

         int   gb_row    = 0;
         auto* gb_layout = new QGridLayout(groupbox);
         groupbox->setLayout(gb_layout);

         {
            auto* explain = new QLabel(tr("You can move the camera along the X, Y, and Z axes, but... which axes? What are they relative to?\n\nIn general, an entity's forward direction is its local Y axis.", "reference frame explanation"));
            explain->setWordWrap(true);
            gb_layout->addWidget(explain, gb_row, 0, 1, 2);
            ++gb_row;
         }

         this->_subwidgets.reference_frames.baseline = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("Default:")), gb_row, 0);
         gb_layout->addWidget(this->_subwidgets.reference_frames.baseline, gb_row, 1);
         ++gb_row;

         this->_subwidgets.reference_frames.selection = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("With a selection:")), gb_row, 0);
         gb_layout->addWidget(this->_subwidgets.reference_frames.selection, gb_row, 1);
         ++gb_row;
      }
      ++row;

      {
         layout->addWidget(new QLabel(tr("Magnitude:")), row, 0);

         auto* wrapper = new QWidget;
         layout->addWidget(wrapper, row, 1);

         auto* wr_layout = new QHBoxLayout;
         wr_layout->setContentsMargins(0, 0, 0, 0);
         wrapper->setLayout(wr_layout);

         this->_subwidgets.magnitude.x = new QDoubleSpinBox;
         this->_subwidgets.magnitude.y = new QDoubleSpinBox;
         this->_subwidgets.magnitude.z = new QDoubleSpinBox;

         wr_layout->addWidget(new QLabel(tr("X", "axis label")));
         wr_layout->addWidget(this->_subwidgets.magnitude.x);
         wr_layout->addWidget(new QLabel(tr("Y", "axis label")));
         wr_layout->addWidget(this->_subwidgets.magnitude.y);
         wr_layout->addWidget(new QLabel(tr("Z", "axis label")));
         wr_layout->addWidget(this->_subwidgets.magnitude.z);
      }
      ++row;

      {
         auto* groupbox = new QGroupBox(tr("Range control mapping"), this);
         layout->addWidget(groupbox, row, 0, 1, 2);

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
         auto& widgets = this->_subwidgets.reference_frames;

         using enum_type = decltype(widgets.baseline)::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::camera, "Camera" },
            item_type{ enum_type::current, "Edit gizmo" },
            item_type{ enum_type::local,   "Selection" },
            item_type{ enum_type::world,   "World" },
         };

         widgets.baseline.addItems(items);
         widgets.selection.addItems(items);

         auto& fields = this->_state.current_options.reference_frames;
         widgets.baseline.beginOneWaySync(fields.baseline);
         widgets.selection.beginOneWaySync(fields.selection);
      }
      {
         auto& widgets = this->_subwidgets.magnitude;
         widgets.x->setRange(-4096, 4096);
         widgets.y->setRange(-4096, 4096);
         widgets.z->setRange(-4096, 4096);
         
         auto& fields = this->_state.current_options.magnitudes;
         cobb::qt::bind(widgets.x, fields.x);
         cobb::qt::bind(widgets.y, fields.y);
         cobb::qt::bind(widgets.z, fields.z);
      }
      {
         auto& widgets = this->_subwidgets.range;
         auto& fields  = this->_state.current_options.range;

         {
            using enum_type = decltype(widgets.x.axis)::value_type;
            using item_type = std::pair<enum_type, const char*>;

            constexpr const auto items = std::array{
               item_type{ enum_type::x, "X" },
               item_type{ enum_type::y, "Y" },
               item_type{ enum_type::z, "Z" },
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

   void move_camera::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         auto& fields = this->_state.current_options.reference_frames;
         auto& widgets = this->_subwidgets.reference_frames;
         widgets.baseline.setValue(fields.baseline);
         widgets.selection.setValue(fields.selection);
      }
      {
         auto& widgets = this->_subwidgets.magnitude;

         const auto blocker_x = QSignalBlocker(widgets.x);
         const auto blocker_y = QSignalBlocker(widgets.y);
         const auto blocker_z = QSignalBlocker(widgets.z);

         widgets.x->setValue(v.magnitudes.x);
         widgets.y->setValue(v.magnitudes.y);
         widgets.z->setValue(v.magnitudes.z);
      }
      {
         const auto& fields = this->_state.current_options.range;
         auto& widgets = this->_subwidgets.range;

         widgets.x.axis.setValue(fields.x.axis);
         widgets.y.axis.setValue(fields.y.axis);

         widgets.x.sign.setValue(fields.x.sign);
         widgets.y.sign.setValue(fields.y.sign);
      }
   }
}