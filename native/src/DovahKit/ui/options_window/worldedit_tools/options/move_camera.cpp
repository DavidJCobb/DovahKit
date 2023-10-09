#include "./move_camera.h"
#include <array>
#include <type_traits>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   move_camera::move_camera(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);

      this->_widget_wrappers.transformFrame = this->ui.transformFrame;
      {
         auto widget = this->_widget_wrappers.transformFrame;

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
         auto* wx = this->ui.magnitudeX;
         auto* wy = this->ui.magnitudeY;
         auto* wz = this->ui.magnitudeZ;

         wx->setRange(-4096, 4096);
         wy->setRange(-4096, 4096);
         wz->setRange(-4096, 4096);
         
         auto& fields = this->_state.current_options.magnitudes;
         cobb::qt::bind(wx, fields.x);
         cobb::qt::bind(wy, fields.y);
         cobb::qt::bind(wz, fields.z);
      }
      this->ui.rangeInputScales->setSyncTarget(&this->_state.current_options.range);
   }

   void move_camera::set_options(const options_type& v) {
      this->_state.current_options = v;
      this->_widget_wrappers.transformFrame.setValue(this->_state.current_options.frame);
      {
         auto* wx = this->ui.magnitudeX;
         auto* wy = this->ui.magnitudeY;
         auto* wz = this->ui.magnitudeZ;

         const auto blocker_x = QSignalBlocker(wx);
         const auto blocker_y = QSignalBlocker(wy);
         const auto blocker_z = QSignalBlocker(wz);

         wx->setValue(v.magnitudes.x);
         wy->setValue(v.magnitudes.y);
         wz->setValue(v.magnitudes.z);
      }
      this->ui.rangeInputScales->reloadFromSyncTarget();
   }

   void move_camera::onRangeInputChanged(dovahkit::subsystems::worldinput::range_input_control ctrl, dovahkit::subsystems::worldinput::range_input_axes axes) {
      this->ui.rangeInputScales->adjustForRangeInput(ctrl, axes);
   }
}