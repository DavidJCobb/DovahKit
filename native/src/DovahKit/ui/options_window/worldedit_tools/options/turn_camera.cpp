#include "./turn_camera.h"
#include <array>
#include <type_traits>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   turn_camera::turn_camera(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);
      {
         auto* widget = this->ui.rangeInputScales;
         widget->setAxisNameOverride(WorldeditToolRangeInputScalesWidget::data_type::axis3D::x, tr("Pitch"));
         widget->setAxisNameOverride(WorldeditToolRangeInputScalesWidget::data_type::axis3D::y, tr("Roll"));
         widget->setAxisNameOverride(WorldeditToolRangeInputScalesWidget::data_type::axis3D::z, tr("Yaw"));
      }

      //
      // Layout done; set up the fields:
      //

      {
         auto* wx = this->ui.magnitudeX;
         auto* wy = this->ui.magnitudeX;
         auto* wz = this->ui.magnitudeX;

         wx->setRange(-360, 360);
         wy->setRange(-360, 360);
         wz->setRange(-360, 360);
         
         auto& fields = this->_state.current_options.magnitudes;
         cobb::qt::bind(wx, fields.x);
         cobb::qt::bind(wy, fields.y);
         cobb::qt::bind(wz, fields.z);
      }
      this->ui.rangeInputScales->setSyncTarget(&this->_state.current_options.range);
   }

   void turn_camera::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         auto* wx = this->ui.magnitudeX;
         auto* wy = this->ui.magnitudeX;
         auto* wz = this->ui.magnitudeX;

         const auto blocker_x = QSignalBlocker(wx);
         const auto blocker_y = QSignalBlocker(wy);
         const auto blocker_z = QSignalBlocker(wz);

         wx->setValue(v.magnitudes.x);
         wy->setValue(v.magnitudes.y);
         wz->setValue(v.magnitudes.z);
      }
      this->ui.rangeInputScales->reloadFromSyncTarget();
   }

   void turn_camera::onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control ctrl, dovahkit::subsystems::worldinput::range_input_axes axes) {
      this->ui.rangeInputScales->adjustForRangeInput(ctrl, axes);
   }
}