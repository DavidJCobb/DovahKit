#include "tool_options_turn_camera.h"
#include "helpers/qt/combobox.h"
#include "helpers/qt/mass_block_signals.h"
#include "helpers/qt/mass_set_visible.h"
#include "editor/subsystems/worldinput/tools/_options.h"

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

namespace {
   static constexpr float max_turn_angle = 359.0F;
}

namespace DK3DToolOptions {
   TurnCamera::TurnCamera(QWidget* parent) : Base(worldinput::id_of_tool<tool_type>(), parent) {
      this->ui.setupUi(this);
      QObject::connect(this, &Base::controlTypeChanged, this, &TurnCamera::_onControlTypeChanged);
      //
      {  // Configure spinboxes
         auto cfg = [this](QDoubleSpinBox* widget) {
            widget->setRange(-max_turn_angle, max_turn_angle);
            widget->setValue(0);
            widget->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
            widget->setSuffix(tr((const char*)u8" \u00B0", "degrees unit"));
            QObject::connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Base::edited); // signal-to-signal
         };
         cfg(this->ui.booleanX);
         cfg(this->ui.booleanZ);
      }
      {  // Configure 3D axes
         auto cfg = [this](QComboBox* widget) {
            widget->clear();
            widget->addItem(tr("Yaw",   "3D axis"), (int)worldinput::camera_turn_axis::yaw);
            widget->addItem(tr("Pitch", "3D axis"), (int)worldinput::camera_turn_axis::pitch);
            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
         };
         cfg(this->ui.scalarAxis);
         cfg(this->ui.vectorXAxis);
         cfg(this->ui.vectorYAxis);
      }
      {  // Configure directions
         QComboBox* widget;
         //
         widget = this->ui.scalarSign;
         widget->clear();
         widget->addItem(tr("Positive", "sign"), (int)worldinput::sign::positive);
         widget->addItem(tr("Negative", "sign"), (int)worldinput::sign::negative);
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
         //
         widget = this->ui.vectorXSign; // TODO: we want different text for different scalar/vector controls
         widget->clear();
         widget->addItem(tr("Positive", "sign"), (int)worldinput::sign::positive);
         widget->addItem(tr("Negative", "sign"), (int)worldinput::sign::negative);
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
         //
         widget = this->ui.vectorYSign; // TODO: we want different text for different scalar/vector controls
         widget->clear();
         widget->addItem(tr("Positive", "sign"), (int)worldinput::sign::positive);
         widget->addItem(tr("Negative", "sign"), (int)worldinput::sign::negative);
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
      }
      //
      this->_onControlTypeChanged();
   }

   void TurnCamera::_onControlTypeChanged() {
      bool button = (this->controlType() == worldinput::control_type::button);
      bool scalar = (this->controlType() == worldinput::control_type::scalar);
      bool vector = (this->controlType() == worldinput::control_type::vector);
      //
      cobb::qt::set_visibility_of(button,
         this->ui.label_booleanX, this->ui.booleanX,
         this->ui.label_booleanZ, this->ui.booleanZ
      );
      cobb::qt::set_visibility_of(scalar,
         this->ui.label_scalar, this->ui.scalarAxis, this->ui.scalarSign
      );
      cobb::qt::set_visibility_of(vector,
         this->ui.label_vectorX, this->ui.vectorXAxis, this->ui.vectorXSign,
         this->ui.label_vectorY, this->ui.vectorYAxis, this->ui.vectorYSign
      );
   }

   void TurnCamera::_readOptions(const worldinput::tools::opaque_option_union& oou) {
      const auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      auto& options = *o;
      //
      auto blockers = cobb::qt::mass_signal_blocker(
         this->ui.booleanX,
         this->ui.booleanZ,
         this->ui.scalarAxis,
         this->ui.scalarSign,
         this->ui.vectorXAxis,
         this->ui.vectorXSign,
         this->ui.vectorYAxis,
         this->ui.vectorYSign
      );
      //
      this->ui.booleanX->setValue(options.magnitudes.pitch);
      this->ui.booleanZ->setValue(options.magnitudes.yaw);
      if (this->controlType() == worldinput::control_type::scalar) {
         cobb::qt::set_combobox_value(this->ui.scalarAxis, options.non_button.input_x);
         cobb::qt::set_combobox_value(this->ui.scalarSign, options.non_button.x_sign);
      } else {
         cobb::qt::set_combobox_value(this->ui.vectorXAxis, options.non_button.input_x);
         cobb::qt::set_combobox_value(this->ui.vectorXSign, options.non_button.x_sign);
         cobb::qt::set_combobox_value(this->ui.vectorYAxis, options.non_button.input_y);
         cobb::qt::set_combobox_value(this->ui.vectorYSign, options.non_button.y_sign);
      }
   }
   void TurnCamera::_writeOptions(worldinput::tools::opaque_option_union& oou) {
      auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      auto& out = *o;
      //
      out.magnitudes.pitch = this->ui.booleanX->value();
      out.magnitudes.yaw   = this->ui.booleanZ->value();
      if (this->controlType() == worldinput::control_type::scalar) {
         out.non_button.input_x = (worldinput::camera_turn_axis)this->ui.scalarAxis->currentData().toInt();
         out.non_button.x_sign  = (worldinput::sign)this->ui.scalarSign->currentData().toInt();
      } else {
         out.non_button.input_x = (worldinput::camera_turn_axis)this->ui.vectorXAxis->currentData().toInt();
         out.non_button.input_y = (worldinput::camera_turn_axis)this->ui.vectorYAxis->currentData().toInt();
         out.non_button.x_sign  = (worldinput::sign)this->ui.vectorXSign->currentData().toInt();
         out.non_button.y_sign  = (worldinput::sign)this->ui.vectorYSign->currentData().toInt();
      }
   }
}