#include "tool_options_turn_camera.h"
#include "helpers/qt/combobox.h"
#include "helpers/qt/mass_set_visible.h"
#include "dk3d/tools/_options.h"

namespace {
   static constexpr float max_turn_angle = 359.0F;
}

namespace DK3DToolOptions {
   TurnCamera::TurnCamera(QWidget* parent) : Base(DK3D::id_of_tool_options<options_type>(), parent) {
      this->ui.setupUi(this);
      QObject::connect(this, &Base::controlTypeChanged, this, &TurnCamera::_onControlTypeChanged);
      //
      {  // Configure spinboxes
         auto cfg = [](QDoubleSpinBox* widget) {
            widget->setRange(-max_turn_angle, max_turn_angle);
            widget->setValue(0);
            widget->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
            widget->setSuffix(tr(" °", "degrees unit"));
         };
         cfg(this->ui.booleanX);
         cfg(this->ui.booleanZ);
      }
      {  // Configure 3D axes
         auto cfg = [](QComboBox* widget) {
            widget->clear();
            widget->addItem(tr("Yaw",   "3D axis"), (int)DK3D::CameraTurnAxis::Yaw);
            widget->addItem(tr("Pitch", "3D axis"), (int)DK3D::CameraTurnAxis::Pitch);
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
         widget->addItem(tr("Positive", "sign"), (int)DK3D::Sign::Positive);
         widget->addItem(tr("Negative", "sign"), (int)DK3D::Sign::Negative);
         //
         widget = this->ui.vectorXSign; // TODO: we want different text for different scalar/vector controls
         widget->clear();
         widget->addItem(tr("Positive", "sign"), (int)DK3D::Sign::Positive);
         widget->addItem(tr("Negative", "sign"), (int)DK3D::Sign::Negative);
         //
         widget = this->ui.vectorYSign; // TODO: we want different text for different scalar/vector controls
         widget->clear();
         widget->addItem(tr("Positive", "sign"), (int)DK3D::Sign::Positive);
         widget->addItem(tr("Negative", "sign"), (int)DK3D::Sign::Negative);
      }
      //
      this->_onControlTypeChanged();
   }

   void TurnCamera::_onControlTypeChanged() {
      bool button = (this->controlType() == DK3D::ControlType::Boolean);
      bool scalar = (this->controlType() == DK3D::ControlType::Scalar);
      bool vector = (this->controlType() == DK3D::ControlType::Vector);
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

   void TurnCamera::_readOptions(const DK3D::tools::opaque_option_union& oou) {
      const auto* o = DK3D::tools::option_union::as<options_type>(oou);
      if (!o)
         return;
      auto& options = *o;
      //
      this->ui.booleanX->setValue(options.magnitudes.pitch);
      this->ui.booleanZ->setValue(options.magnitudes.yaw);
      if (this->controlType() == DK3D::ControlType::Scalar) {
         cobb::qt::set_combobox_value(this->ui.scalarAxis, options.non_button.input_x);
         cobb::qt::set_combobox_value(this->ui.scalarSign, options.non_button.x_sign);
      } else {
         cobb::qt::set_combobox_value(this->ui.vectorXAxis, options.non_button.input_x);
         cobb::qt::set_combobox_value(this->ui.vectorXSign, options.non_button.x_sign);
         cobb::qt::set_combobox_value(this->ui.vectorYAxis, options.non_button.input_y);
         cobb::qt::set_combobox_value(this->ui.vectorYSign, options.non_button.y_sign);
      }
   }
   void TurnCamera::_writeOptions(DK3D::tools::opaque_option_union& oou) {
      auto* o = DK3D::tools::option_union::as<options_type>(oou);
      if (!o)
         return;
      auto& out = *o;
      //
      out.magnitudes.pitch = this->ui.booleanX->value();
      out.magnitudes.yaw   = this->ui.booleanZ->value();
      if (this->controlType() == DK3D::ControlType::Scalar) {
         out.non_button.input_x = (DK3D::CameraTurnAxis)this->ui.scalarAxis->currentData().toInt();
         out.non_button.x_sign  = (DK3D::Sign)this->ui.scalarSign->currentData().toInt();
      } else {
         out.non_button.input_x = (DK3D::CameraTurnAxis)this->ui.vectorXAxis->currentData().toInt();
         out.non_button.input_y = (DK3D::CameraTurnAxis)this->ui.vectorYAxis->currentData().toInt();
         out.non_button.x_sign  = (DK3D::Sign)this->ui.vectorXSign->currentData().toInt();
         out.non_button.y_sign  = (DK3D::Sign)this->ui.vectorYSign->currentData().toInt();
      }
   }
}