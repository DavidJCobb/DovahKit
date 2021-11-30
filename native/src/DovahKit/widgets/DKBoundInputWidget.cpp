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
      auto* wrapper = sw.boolean.wrapper = new QWidget(this);
      auto* layout  = new QBoxLayout(QBoxLayout::Direction::LeftToRight, wrapper);
      auto* picker  = sw.boolean.control = new DKKeyPickerWidget(this);
      auto* type    = sw.boolean.mod     = new QComboBox(this);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->addWidget(type);
      layout->addWidget(picker);
      //
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
      widget->addItem(tr("Left Trigger",      "XInput button"), (int)DK3D::XInputKey::LT);
      widget->addItem(tr("Right Trigger",     "XInput button"), (int)DK3D::XInputKey::RT);
      widget->addItem(tr("Left Bumper",       "XInput button"), (int)DK3D::XInputKey::LB);
      widget->addItem(tr("Right Bumper",      "XInput button"), (int)DK3D::XInputKey::RB);
      layout->addWidget(widget);
   }
   {
      auto* wrapper = sw.scalar.wrapper = new QWidget(this);
      auto* layout  = new QBoxLayout(QBoxLayout::Direction::LeftToRight, wrapper);
      sw.scalar.control = new QComboBox(this);
      sw.scalar.sign    = new QComboBox(this);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->addWidget(sw.scalar.control);
      layout->addWidget(sw.scalar.sign);
   }
   {
      auto* wrapper = sw.vector.wrapper = new QWidget(this);
      auto* layout  = new QBoxLayout(QBoxLayout::Direction::LeftToRight, wrapper);
      sw.vector.control = new QComboBox(this);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->addWidget(sw.vector.control);
   }
   //
   sw.boolean.label = new QLabel(tr("Interaction:"), this);
   sw.scalar.label  = new QLabel(tr("Control"), this);
   sw.vector.label  = new QLabel(tr("Control"), this);
   //
   layout->addWidget(new QLabel(tr("Type:"), this), 0, 0);
   layout->addWidget(sw.control_type, 0, 1);
   //
   // Overlap widgets; we'll only show one set at a time:
   //
   layout->addWidget(sw.boolean.label,   1, 0);
   layout->addWidget(sw.boolean.wrapper, 1, 1);
   layout->addWidget(sw.scalar.label,    1, 0);
   layout->addWidget(sw.scalar.wrapper,  1, 1);
   layout->addWidget(sw.vector.label,    1, 0);
   layout->addWidget(sw.vector.wrapper,  1, 1);
   //
   this->_onControlTypeChange();
   this->_onInputDeviceChange();
}

DK3D::BoundInput DKBoundInputWidget::value() const {
   DK3D::BoundInput out;
   //
   switch ((ControlType)this->subwidgets.control_type->currentData().toInt()) {
      case ControlType::Boolean:
         {
            out.boolean.type = (DK3D::BooleanInputMod)this->subwidgets.boolean.mod->currentData().toInt();
            switch (this->inputDevice()) {
               case InputDevice::KeyboardMouse:
                  {
                     auto keys = this->subwidgets.boolean.control->keys();
                     if (!keys.empty())
                        out.boolean.key = keys[0];
                  }
                  break;
               case InputDevice::XInput:
                  out.boolean.gamepad.button = (DK3D::XInputKey)this->subwidgets.boolean.xinput->currentData().toInt();
                  break;
            }
            return out;
         }
         break;
      case ControlType::Scalar:
         {
            out.scalar.input = (DK3D::ScalarControl)this->subwidgets.scalar.control->currentData(ControlRole).toInt();
            out.scalar.axis  = (DK3D::Axis2D)this->subwidgets.scalar.control->currentData(AxisRole).toInt();
         }
         break;
      case ControlType::Vector:
         {
            out.vector.input = (DK3D::VectorControl)this->subwidgets.vector.control->currentData().toInt();
         }
         break;
   }
   //
   return out;
}
void DKBoundInputWidget::setInputDevice(InputDevice d) {
   if (this->state.inputDevice == d)
      return;
   this->state.inputDevice = d;
   this->_onInputDeviceChange();
}
void DKBoundInputWidget::setValue(const DK3D::BoundInput& src) {
   const auto blocker0 = QSignalBlocker(this->subwidgets.control_type);
   if (src.is_boolean()) {
      this->subwidgets.control_type->setCurrentIndex(this->subwidgets.control_type->findData((int)ControlType::Boolean));
      this->_onControlTypeChange();
      //
      auto& b  = src.boolean;
      auto& sw = this->subwidgets.boolean;
      sw.mod->setCurrentIndex(sw.mod->findData((int)b.type));
      if (!b.key.empty()) {
         sw.control->setKeys({ b.key });
      } else if (b.mouse.button != Qt::MouseButton::NoButton) {
         //
         // TODO
         //
      } else if (b.gamepad.button != DK3D::XInputKey::None) {
         sw.xinput->setCurrentIndex(sw.xinput->findData((int)b.gamepad.button));
      }
      return;
   }
   if (src.is_scalar()) {
      this->subwidgets.control_type->setCurrentIndex(this->subwidgets.control_type->findData((int)ControlType::Scalar));
      this->_onControlTypeChange();
      //
      auto& s  = src.scalar;
      auto& sw = this->subwidgets.scalar;
      //
      auto* widget = sw.control;
      int   size   = widget->count();
      for (int i = 0; i < size; ++i) {
         if ((DK3D::ScalarControl)widget->itemData(i, ControlRole).toInt() != s.input)
            continue;
         switch (s.input) {
            case DK3D::ScalarControl::XInput_LS:
            case DK3D::ScalarControl::XInput_RS:
            case DK3D::ScalarControl::MouseMove:
               if ((DK3D::Axis2D)widget->itemData(i, AxisRole).toInt() != s.axis)
                  continue;
               break;
         }
         widget->setCurrentIndex(i);
         break;
      }
      return;
   }
   if (src.is_vector()) {
      this->subwidgets.control_type->setCurrentIndex(this->subwidgets.control_type->findData((int)ControlType::Vector));
      this->_onControlTypeChange();
      //
      auto& v  = src.vector;
      auto& sw = this->subwidgets.vector;
      //
      sw.control->setCurrentIndex(sw.control->findData((int)v.input));
      return;
   }
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
   sw.boolean.wrapper->setHidden(!button);
   sw.scalar.label->setHidden(!scalar);
   sw.scalar.wrapper->setHidden(!scalar);
   sw.vector.label->setHidden(!vector);
   sw.vector.wrapper->setHidden(!vector);
}
void DKBoundInputWidget::_onInputDeviceChange() {
   auto id = this->inputDevice();
   //
   {
      auto* widget = this->subwidgets.boolean.control;
      auto* button = this->subwidgets.boolean.xinput;
      widget->setHidden(id != InputDevice::KeyboardMouse);
      button->setHidden(id != InputDevice::XInput);
   }
   {
      struct _VectorToScalar {
         const char* translation;
         DK3D::ScalarControl control;
         DK3D::Axis2D axis;
      };
      //
      auto*      widget  = this->subwidgets.scalar.control;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (id == InputDevice::KeyboardMouse) {
         constexpr auto vector_to_scalar_entries = std::array{
            _VectorToScalar{ "Mouse X", DK3D::ScalarControl::MouseMove, DK3D::Axis2D::X },
            _VectorToScalar{ "Mouse Y", DK3D::ScalarControl::MouseMove, DK3D::Axis2D::Y },
         };
         //
         for (size_t i = 0; i < vector_to_scalar_entries.size(); ++i) {
            auto& entry = vector_to_scalar_entries[i];
            widget->addItem(tr(entry.translation));
            widget->setItemData(i, (int)entry.control, ControlRole);
            widget->setItemData(i, (int)entry.axis,    AxisRole);
         }
      } else if (id == InputDevice::XInput) {
         constexpr auto vector_to_scalar_entries = std::array{
            _VectorToScalar{ "Left Stick X",  DK3D::ScalarControl::XInput_LS, DK3D::Axis2D::X },
            _VectorToScalar{ "Left Stick Y",  DK3D::ScalarControl::XInput_LS, DK3D::Axis2D::Y },
            _VectorToScalar{ "Right Stick X", DK3D::ScalarControl::XInput_RS, DK3D::Axis2D::X },
            _VectorToScalar{ "Right Stick Y", DK3D::ScalarControl::XInput_RS, DK3D::Axis2D::Y },
         };
         //
         for (size_t i = 0; i < vector_to_scalar_entries.size(); ++i) {
            auto& entry = vector_to_scalar_entries[i];
            widget->addItem(tr(entry.translation));
            widget->setItemData(i, (int)entry.control, ControlRole);
            widget->setItemData(i, (int)entry.axis,    AxisRole);
         }
         widget->addItem(tr("Left Trigger"),  (int)DK3D::ScalarControl::XInput_LT);
         widget->addItem(tr("Right Trigger"), (int)DK3D::ScalarControl::XInput_RT);
      }
   }
   {
      auto*      widget  = this->subwidgets.vector.control;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (id == InputDevice::KeyboardMouse) {
         widget->addItem(tr("Mouse Move"), (int)DK3D::VectorControl::MouseMove);
      } else if (id == InputDevice::XInput) {
         widget->addItem(tr("Left Stick"),  (int)DK3D::VectorControl::XInput_LS);
         widget->addItem(tr("Right Stick"), (int)DK3D::VectorControl::XInput_RS);
      }
   }
}