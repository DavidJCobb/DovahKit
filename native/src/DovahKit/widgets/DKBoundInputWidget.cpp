#include "DKBoundInputWidget.h"
#include <QGridLayout>
#include <QLabel>
#include "helpers/qt/combobox.h"
#if !defined(QT_DESIGNER_LIB)
   #include "dk3d/enums/button_press_type.h"
#endif

DKBoundInputWidget::DKBoundInputWidget(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   auto& sw = this->subwidgets;
   {
      auto* widget = sw.control_type = new QComboBox(this);
      widget->addItem(tr("Button or key"), (int)DK3D::control_type::button);
      widget->addItem(tr("Move (1D)"),     (int)DK3D::control_type::scalar);
      widget->addItem(tr("Move (2D)"),     (int)DK3D::control_type::vector);
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
      #if !defined(QT_DESIGNER_LIB)
         type->addItem(tr("Tap"),   (int)DK3D::button_press_type::tap);
         type->addItem(tr("Hold"),  (int)DK3D::button_press_type::hold);
         type->addItem(tr("While"), (int)DK3D::button_press_type::while_down);
      #endif
      //
      auto* widget = sw.boolean.xinput = new QComboBox(this);
      #if !defined(QT_DESIGNER_LIB)
         widget->addItem(tr("A",                 "XInput button"), (int)DK3D::inputs::xinput_button::A);
         widget->addItem(tr("B",                 "XInput button"), (int)DK3D::inputs::xinput_button::B);
         widget->addItem(tr("X",                 "XInput button"), (int)DK3D::inputs::xinput_button::X);
         widget->addItem(tr("Y",                 "XInput button"), (int)DK3D::inputs::xinput_button::Y);
         widget->addItem(tr("Back",              "XInput button"), (int)DK3D::inputs::xinput_button::Back);
         widget->addItem(tr("Start",             "XInput button"), (int)DK3D::inputs::xinput_button::Start);
         widget->addItem(tr("D-Pad Up",          "XInput button"), (int)DK3D::inputs::xinput_button::DPadUp);
         widget->addItem(tr("D-Pad Down",        "XInput button"), (int)DK3D::inputs::xinput_button::DPadDown);
         widget->addItem(tr("D-Pad Left",        "XInput button"), (int)DK3D::inputs::xinput_button::DPadLeft);
         widget->addItem(tr("D-Pad Right",       "XInput button"), (int)DK3D::inputs::xinput_button::DPadRight);
         widget->addItem(tr("Left Stick Click",  "XInput button"), (int)DK3D::inputs::xinput_button::LS);
         widget->addItem(tr("Right Stick Click", "XInput button"), (int)DK3D::inputs::xinput_button::RS);
         widget->addItem(tr("Left Trigger",      "XInput button"), (int)DK3D::inputs::xinput_button::LT);
         widget->addItem(tr("Right Trigger",     "XInput button"), (int)DK3D::inputs::xinput_button::RT);
         widget->addItem(tr("Left Bumper",       "XInput button"), (int)DK3D::inputs::xinput_button::LB);
         widget->addItem(tr("Right Bumper",      "XInput button"), (int)DK3D::inputs::xinput_button::RB);
      #endif
      layout->addWidget(widget);
      //
      QObject::connect(picker, &DKKeyPickerWidget::valueChanged, this, &DKBoundInputWidget::_sendValueChanged);
      QObject::connect(type,   QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DKBoundInputWidget::_sendValueChanged);
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DKBoundInputWidget::_sendValueChanged);
   }
   {
      auto* wrapper = sw.scalar.wrapper = new QWidget(this);
      auto* layout  = new QBoxLayout(QBoxLayout::Direction::LeftToRight, wrapper);
      sw.scalar.control = new QComboBox(this);
      sw.scalar.sign    = new QComboBox(this);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->addWidget(sw.scalar.control);
      layout->addWidget(sw.scalar.sign);
      //
      QObject::connect(sw.scalar.control, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DKBoundInputWidget::_sendValueChanged);
      QObject::connect(sw.scalar.sign,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DKBoundInputWidget::_sendValueChanged);
   }
   {
      auto* wrapper = sw.vector.wrapper = new QWidget(this);
      auto* layout  = new QBoxLayout(QBoxLayout::Direction::LeftToRight, wrapper);
      sw.vector.control = new QComboBox(this);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->addWidget(sw.vector.control);
      //
      QObject::connect(sw.vector.control, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DKBoundInputWidget::_sendValueChanged);
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

DK3D::inputs::bound_input DKBoundInputWidget::value() const {
   DK3D::inputs::bound_input out;
   //
   #if !defined(QT_DESIGNER_LIB)
   switch ((DK3D::control_type)this->subwidgets.control_type->currentData().toInt()) {
      case DK3D::control_type::button:
         {
            out.button.press_type = (DK3D::button_press_type)this->subwidgets.boolean.mod->currentData().toInt();
            switch (this->inputDevice()) {
               case DK3D::input_device_type::keyboard_mouse:
                  {
                     auto keys = this->subwidgets.boolean.control->keys();
                     if (!keys.empty())
                        out.button.key = keys[0];
                  }
                  break;
               case DK3D::input_device_type::xinput:
                  out.button.gamepad = (DK3D::inputs::xinput_button)this->subwidgets.boolean.xinput->currentData().toInt();
                  break;
            }
            return out;
         }
         break;
      case DK3D::control_type::scalar:
         {
            out.scalar.input = (DK3D::scalar_control)this->subwidgets.scalar.control->currentData(ControlRole).toInt();
            out.scalar.axis  = (DK3D::axis2D)this->subwidgets.scalar.control->currentData(AxisRole).toInt();
         }
         break;
      case DK3D::control_type::vector:
         {
            out.vector.input = (DK3D::vector_control)this->subwidgets.vector.control->currentData().toInt();
         }
         break;
   }
   #endif
   //
   return out;
}
void DKBoundInputWidget::setInputDevice(DK3D::input_device_type d) {
   if (this->state.inputDevice == d)
      return;
   this->state.inputDevice = d;
   this->_onInputDeviceChange();
}
void DKBoundInputWidget::setValue(const DK3D::inputs::bound_input& src) {
   #if !defined(QT_DESIGNER_LIB)
   const auto blocker0 = QSignalBlocker(this->subwidgets.control_type);
   if (src.is_button()) {
      cobb::qt::set_combobox_value(this->subwidgets.control_type, DK3D::control_type::button);
      this->_onControlTypeChange();
      //
      auto& b  = src.button;
      auto& sw = this->subwidgets.boolean;
      cobb::qt::set_combobox_value(sw.mod, b.press_type);
      if (!b.key.empty()) {
         sw.control->setKeys({ b.key });
      } else if (b.mouse != Qt::MouseButton::NoButton) {
         //
         // TODO
         //
      } else if (b.gamepad != DK3D::inputs::xinput_button::None) {
         cobb::qt::set_combobox_value(sw.xinput, b.gamepad);
      }
      return;
   } else {
      auto& sw = this->subwidgets.boolean;
      sw.control->setKeys({});
      cobb::qt::set_combobox_value(sw.mod,    DK3D::button_press_type::tap);
      cobb::qt::set_combobox_value(sw.xinput, DK3D::inputs::xinput_button::A);
   }
   if (src.is_scalar()) {
      cobb::qt::set_combobox_value(this->subwidgets.control_type, DK3D::control_type::scalar);
      this->_onControlTypeChange();
      //
      auto& s  = src.scalar;
      auto& sw = this->subwidgets.scalar;
      //
      auto* widget = sw.control;
      int   size   = widget->count();
      for (int i = 0; i < size; ++i) {
         if ((DK3D::scalar_control)widget->itemData(i, ControlRole).toInt() != s.input)
            continue;
         switch (s.input) {
            case DK3D::scalar_control::xinput_ls:
            case DK3D::scalar_control::xinput_rs:
            case DK3D::scalar_control::mouse_move:
               if ((DK3D::axis2D)widget->itemData(i, AxisRole).toInt() != s.axis)
                  continue;
               break;
         }
         widget->setCurrentIndex(i);
         break;
      }
      return;
   }
   if (src.is_vector()) {
      cobb::qt::set_combobox_value(this->subwidgets.control_type, DK3D::control_type::vector);
      this->_onControlTypeChange();
      //
      auto& v  = src.vector;
      auto& sw = this->subwidgets.vector;
      cobb::qt::set_combobox_value(sw.control, v.input);
      return;
   } else {
      auto& sw = this->subwidgets.vector;
      switch (this->inputDevice()) {
         case DK3D::input_device_type::keyboard_mouse:
            cobb::qt::set_combobox_value(sw.control, DK3D::vector_control::mouse_move);
            break;
         case DK3D::input_device_type::xinput:
            cobb::qt::set_combobox_value(sw.control, DK3D::vector_control::xinput_ls);
            break;
      }
   }
   #endif
}

void DKBoundInputWidget::_onControlTypeChange() {
   bool button = false;
   bool scalar = false;
   bool vector = false;
   switch ((DK3D::control_type)this->subwidgets.control_type->currentData().toInt()) {
      case DK3D::control_type::button:
         button = true;
         break;
      case DK3D::control_type::scalar:
         scalar = true;
         break;
      case DK3D::control_type::vector:
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
   //
   this->_sendValueChanged();
}
void DKBoundInputWidget::_onInputDeviceChange() {
   auto id = this->inputDevice();
   //
   {
      auto* widget = this->subwidgets.boolean.control;
      auto* button = this->subwidgets.boolean.xinput;
      widget->setHidden(id != DK3D::input_device_type::keyboard_mouse);
      button->setHidden(id != DK3D::input_device_type::xinput);
   }
   #if !defined(QT_DESIGNER_LIB)
   {
      struct _VectorToScalar {
         const char* translation;
         DK3D::scalar_control control;
         DK3D::axis2D axis;
      };
      //
      auto*      widget  = this->subwidgets.scalar.control;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (id == DK3D::input_device_type::keyboard_mouse) {
         constexpr auto vector_to_scalar_entries = std::array{
            _VectorToScalar{ "Mouse X", DK3D::scalar_control::mouse_move, DK3D::axis2D::x },
            _VectorToScalar{ "Mouse Y", DK3D::scalar_control::mouse_move, DK3D::axis2D::y },
         };
         //
         for (size_t i = 0; i < vector_to_scalar_entries.size(); ++i) {
            auto& entry = vector_to_scalar_entries[i];
            widget->addItem(tr(entry.translation));
            widget->setItemData(i, (int)entry.control, ControlRole);
            widget->setItemData(i, (int)entry.axis,    AxisRole);
         }
      } else if (id == DK3D::input_device_type::xinput) {
         constexpr auto vector_to_scalar_entries = std::array{
            _VectorToScalar{ "Left Stick X",  DK3D::scalar_control::xinput_ls, DK3D::axis2D::x },
            _VectorToScalar{ "Left Stick Y",  DK3D::scalar_control::xinput_ls, DK3D::axis2D::y },
            _VectorToScalar{ "Right Stick X", DK3D::scalar_control::xinput_rs, DK3D::axis2D::x },
            _VectorToScalar{ "Right Stick Y", DK3D::scalar_control::xinput_rs, DK3D::axis2D::y },
         };
         //
         for (size_t i = 0; i < vector_to_scalar_entries.size(); ++i) {
            auto& entry = vector_to_scalar_entries[i];
            widget->addItem(tr(entry.translation));
            widget->setItemData(i, (int)entry.control, ControlRole);
            widget->setItemData(i, (int)entry.axis,    AxisRole);
         }
         widget->addItem(tr("Left Trigger"),  (int)DK3D::scalar_control::xinput_lt);
         widget->addItem(tr("Right Trigger"), (int)DK3D::scalar_control::xinput_rt);
      }
   }
   {
      auto*      widget  = this->subwidgets.vector.control;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (id == DK3D::input_device_type::keyboard_mouse) {
         widget->addItem(tr("Mouse Move"), (int)DK3D::vector_control::mouse_move);
      } else if (id == DK3D::input_device_type::xinput) {
         widget->addItem(tr("Left Stick"),  (int)DK3D::vector_control::xinput_ls);
         widget->addItem(tr("Right Stick"), (int)DK3D::vector_control::xinput_rs);
      }
   }
   #endif
   this->_sendValueChanged();
}
void DKBoundInputWidget::_sendValueChanged() {
   emit this->valueChanged(this->value());
}