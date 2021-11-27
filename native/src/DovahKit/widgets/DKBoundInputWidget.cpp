#include "DKBoundInputWidget.h"
#include <QGridLayout>
#include <QLabel>

DKBoundInputWidget::DKBoundInputWidget(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   auto& sw = this->subwidgets;
   {
      auto* widget = sw.control_type = new QComboBox(this);
      widget->addItem(tr("Button or key"), (int)ControlType::Boolean);
      widget->addItem(tr("Move (1D)"),     (int)ControlType::Scalar);
      widget->addItem(tr("Move (2D)"),     (int)ControlType::Vector);
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DKBoundInputWidget::_onControlTypeChange);
   }
   {
      auto* picker = sw.boolean.control = new DKKeyPickerWidget(this);
      auto* type   = sw.boolean.mod     = new QComboBox(this);
      picker->setAllowKeyCombinations(false);
      type->addItem(tr("Tap"),   (int)DK3D::BooleanInputMod::Tap);
      type->addItem(tr("Hold"),  (int)DK3D::BooleanInputMod::Hold);
      type->addItem(tr("While"), (int)DK3D::BooleanInputMod::While);
      //
      auto* widget = sw.boolean.xinput = new QComboBox(this);
      widget->addItem(tr("A",                 "XInput button"), (int)DK3D::XInputKey::A);
      widget->addItem(tr("B",                 "XInput button"), (int)DK3D::XInputKey::B);
      widget->addItem(tr("X",                 "XInput button"), (int)DK3D::XInputKey::X);
      widget->addItem(tr("Y",                 "XInput button"), (int)DK3D::XInputKey::Y);
      widget->addItem(tr("Back",              "XInput button"), (int)DK3D::XInputKey::Back);
      widget->addItem(tr("Start",             "XInput button"), (int)DK3D::XInputKey::Start);
      widget->addItem(tr("D-Pad Up",          "XInput button"), (int)DK3D::XInputKey::DPadUp);
      widget->addItem(tr("D-Pad Down",        "XInput button"), (int)DK3D::XInputKey::DPadDown);
      widget->addItem(tr("D-Pad Left",        "XInput button"), (int)DK3D::XInputKey::DPadLeft);
      widget->addItem(tr("D-Pad Right",       "XInput button"), (int)DK3D::XInputKey::DPadRight);
      widget->addItem(tr("Left Stick Click",  "XInput button"), (int)DK3D::XInputKey::LS);
      widget->addItem(tr("Right Stick Click", "XInput button"), (int)DK3D::XInputKey::RS);
      //widget->addItem(tr("A", "XInput button"), (int)DK3D::XInputKey::LT); // TODO
      //widget->addItem(tr("A", "XInput button"), (int)DK3D::XInputKey::RT); // TODO
      widget->addItem(tr("Left Bumper",       "XInput button"), (int)DK3D::XInputKey::LB);
      widget->addItem(tr("Right Bumper",      "XInput button"), (int)DK3D::XInputKey::RB);
   }
   sw.scalar.control = new QComboBox(this);
   sw.scalar.sign    = new QComboBox(this);
   sw.vector.control = new QComboBox(this);
   //
   sw.boolean.label = new QLabel(tr("Interaction:"), this);
   sw.scalar.label  = new QLabel(tr("Control"), this);
   sw.vector.label  = new QLabel(tr("Control"), this);
   //
   layout->addWidget(new QLabel(tr("Type:"), this), 0, 0);
   //
   // Overlap widgets; we'll only show one set at a time:
   //
   layout->addWidget(sw.boolean.label,   1, 0);
   layout->addWidget(sw.boolean.mod,     1, 1);
   layout->addWidget(sw.boolean.control, 1, 2);
   layout->addWidget(sw.boolean.xinput,  1, 2);
   layout->addWidget(sw.scalar.label,    1, 0);
   layout->addWidget(sw.scalar.control,  1, 1);
   layout->addWidget(sw.scalar.sign,     1, 2);
   layout->addWidget(sw.vector.label,    1, 0);
   layout->addWidget(sw.vector.control,  1, 1, 1, 2);
   //
   this->_onControlTypeChange();
   this->_onInputDeviceChange();
}

void DKBoundInputWidget::_onControlTypeChange() {
   bool button = false;
   bool scalar = false;
   bool vector = false;
   switch ((ControlType)this->subwidgets.control_type->currentData().toInt()) {
      case ControlType::Boolean:
         button = true;
         break;
      case ControlType::Scalar:
         scalar = true;
         break;
      case ControlType::Vector:
         vector = true;
         break;
   }
   //
   auto& sw = this->subwidgets;
   sw.boolean.label->setHidden(!button);
   sw.boolean.mod->setHidden(!button);
   sw.boolean.control->setHidden(!button || (this->state.inputDevice != InputDevice::KeyboardMouse));
   sw.boolean.xinput->setHidden(!button || (this->state.inputDevice != InputDevice::XInput));
   sw.scalar.label->setHidden(!scalar);
   sw.scalar.control->setHidden(!scalar);
   sw.scalar.sign->setHidden(!scalar);
   sw.vector.label->setHidden(!vector);
   sw.vector.control->setHidden(!vector);
}
void DKBoundInputWidget::_onInputDeviceChange() {
   bool button = false;
   bool scalar = false;
   bool vector = false;
   switch ((ControlType)this->subwidgets.control_type->currentData().toInt()) {
      case ControlType::Boolean:
         button = true;
         break;
      case ControlType::Scalar:
         scalar = true;
         break;
      case ControlType::Vector:
         vector = true;
         break;
   }
   //
   auto id = this->inputDevice();
   //
   if (button) {
      auto* widget = this->subwidgets.boolean.control;
      auto* button = this->subwidgets.boolean.xinput;
      widget->setHidden(id != InputDevice::KeyboardMouse);
      button->setHidden(id != InputDevice::XInput);
   } else if (scalar) {
      auto*      widget  = this->subwidgets.scalar.control;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (id == InputDevice::KeyboardMouse) {
         widget->addItem(tr("Mouse X"), DK3D::ScalarControl::MouseMove); // TODO: need a pair of ScalarControl and Axis2D
         widget->addItem(tr("Mouse Y"), DK3D::ScalarControl::MouseMove); // TODO: need a pair of ScalarControl and Axis2D
      } else if (id == InputDevice::XInput) {
         widget->addItem(tr("Left Stick X"),  DK3D::ScalarControl::XInput_LS); // TODO: need a pair of ScalarControl and Axis2D
         widget->addItem(tr("Left Stick Y"),  DK3D::ScalarControl::XInput_LS); // TODO: need a pair of ScalarControl and Axis2D
         widget->addItem(tr("Right Stick X"), DK3D::ScalarControl::XInput_RS); // TODO: need a pair of ScalarControl and Axis2D
         widget->addItem(tr("Right Stick X"), DK3D::ScalarControl::XInput_RS); // TODO: need a pair of ScalarControl and Axis2D
         widget->addItem(tr("Left Trigger"),  DK3D::ScalarControl::XInput_LT);
         widget->addItem(tr("Right Trigger"), DK3D::ScalarControl::XInput_RT);
      }
   } else if (vector) {
      auto*      widget  = this->subwidgets.vector.control;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (id == InputDevice::KeyboardMouse) {
         widget->addItem(tr("Mouse Move"), (int)DK3D::VectorControl::MouseMove);
      } else if (id == InputDevice::XInput) {
         widget->addItem(tr("Left Stick X"), (int)DK3D::VectorControl::XInput_LS);
         widget->addItem(tr("Right Stick X"), (int)DK3D::VectorControl::XInput_RS);
      }
   }
}