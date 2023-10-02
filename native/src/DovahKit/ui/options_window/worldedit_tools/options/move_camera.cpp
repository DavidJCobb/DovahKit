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
         auto* groupbox = new QGroupBox(tr("Reference frame"), this);
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

         this->_subwidgets.reference_frame = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("Frame:")), gb_row, 0);
         gb_layout->addWidget(this->_subwidgets.reference_frame, gb_row, 1);
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
         auto* widget = new WorldeditToolRangeInputScalesWidget(this);
         this->_subwidgets.range = widget;
         layout->addWidget(widget, row, 0, 1, 2);
      }
      ++row;

      //
      // Layout done; set up the fields:
      //

      {
         auto widget = this->_subwidgets.reference_frame;

         using enum_type = decltype(widget)::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::camera,  "Camera" },
            item_type{ enum_type::current, "Edit gizmo" },
            item_type{ enum_type::local,   "Selection" },
            item_type{ enum_type::world,   "World" },
         };

         widget.addItems(items);
         widget.beginOneWaySync(this->_state.current_options.frame);
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
      this->_subwidgets.range->setSyncTarget(&this->_state.current_options.range);
   }

   void move_camera::set_options(const options_type& v) {
      this->_state.current_options = v;
      this->_subwidgets.reference_frame.setValue(this->_state.current_options.frame);
      {
         auto& widgets = this->_subwidgets.magnitude;

         const auto blocker_x = QSignalBlocker(widgets.x);
         const auto blocker_y = QSignalBlocker(widgets.y);
         const auto blocker_z = QSignalBlocker(widgets.z);

         widgets.x->setValue(v.magnitudes.x);
         widgets.y->setValue(v.magnitudes.y);
         widgets.z->setValue(v.magnitudes.z);
      }
      this->_subwidgets.range->reloadFromSyncTarget();
   }
}