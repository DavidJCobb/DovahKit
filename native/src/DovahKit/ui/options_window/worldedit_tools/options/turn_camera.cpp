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
      layout->setContentsMargins(0, 0, 0, 0);
      this->setLayout(layout);

      int row = 0;

      {
         layout->addWidget(new QLabel(tr("Magnitude:")), row, 0);

         auto* wrapper = new QWidget;
         layout->addWidget(wrapper, row, 1);

         auto* wr_layout = new QHBoxLayout;
         wr_layout->setContentsMargins(0, 0, 0, 0);
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
         auto* widget = new WorldeditToolRangeInputScalesWidget(this);
         this->_subwidgets.range = widget;
         layout->addWidget(widget, row, 0, 1, 2);

         widget->setAxisNameOverride(WorldeditToolRangeInputScalesWidget::data_type::axis3D::x, tr("Pitch"));
         widget->setAxisNameOverride(WorldeditToolRangeInputScalesWidget::data_type::axis3D::y, tr("Roll"));
         widget->setAxisNameOverride(WorldeditToolRangeInputScalesWidget::data_type::axis3D::z, tr("Yaw"));
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
         cobb::qt::bind(widgets.yaw,   fields.z);
         cobb::qt::bind(widgets.pitch, fields.x);
      }
      this->_subwidgets.range->setSyncTarget(&this->_state.current_options.range);
   }

   void turn_camera::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         auto& widgets = this->_subwidgets.magnitude;

         const auto blocker_z = QSignalBlocker(widgets.yaw);
         const auto blocker_x = QSignalBlocker(widgets.pitch);

         widgets.yaw->setValue(v.magnitudes.z);
         widgets.pitch->setValue(v.magnitudes.x);
      }
      this->_subwidgets.range->reloadFromSyncTarget();
   }
}