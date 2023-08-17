#include "./DKWorldinputButtonPickDialog.h"
#include <array>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPushButton>
#include "helpers/keyboard/key.h"

DKWorldinputButtonPickDialog::DKWorldinputButtonPickDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->setWindowFlag(Qt::WindowContextHelpButtonHint);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);

   this->ui.mouseEdit->installEventFilter(this);
   this->ui.mouseEdit->setContextMenuPolicy(Qt::PreventContextMenu);
   this->ui.keyEdit->setMaxLength(128);
   this->ui.keyEdit->installEventFilter(this);
   this->ui.keyEdit->setContextMenuPolicy(Qt::PreventContextMenu);

   {
      auto* widget = this->ui.mousePicker;
      widget->clear();
      
      constexpr const auto buttons = std::array{
         Qt::MouseButton::LeftButton,
         Qt::MouseButton::RightButton,
         Qt::MouseButton::MidButton,
         Qt::MouseButton::ExtraButton1,
         Qt::MouseButton::ExtraButton2,
         Qt::MouseButton::ExtraButton3,
         Qt::MouseButton::ExtraButton4,
         Qt::MouseButton::ExtraButton5,
         Qt::MouseButton::ExtraButton6,
         Qt::MouseButton::ExtraButton7,
         Qt::MouseButton::ExtraButton8,
         Qt::MouseButton::ExtraButton9,
         Qt::MouseButton::ExtraButton10,
         Qt::MouseButton::ExtraButton11,
         Qt::MouseButton::ExtraButton12,
         Qt::MouseButton::ExtraButton13,
         Qt::MouseButton::ExtraButton14,
         Qt::MouseButton::ExtraButton15,
         Qt::MouseButton::ExtraButton16,
         Qt::MouseButton::ExtraButton17,
         Qt::MouseButton::ExtraButton18,
         Qt::MouseButton::ExtraButton19,
         Qt::MouseButton::ExtraButton20,
         Qt::MouseButton::ExtraButton21,
         Qt::MouseButton::ExtraButton22,
         Qt::MouseButton::ExtraButton23,
         Qt::MouseButton::ExtraButton24,
      };
      widget->addItem(tr("None"), (int)Qt::MouseButton::NoButton);
      for (const auto b : buttons) {
         widget->addItem(nameOf(b), (int)b);
      }
      
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget]() {
         this->setButton(Button{
            .mouse = (Qt::MouseButton)widget->currentData().toInt(),
         });
      });
   }

   {
      auto* widget = this->ui.gamepadPicker;
      widget->clear();

      constexpr const auto buttons = std::array{
         GamepadButton::a,
         GamepadButton::b,
         GamepadButton::x,
         GamepadButton::y,
         GamepadButton::start,
         GamepadButton::back,
         GamepadButton::ls,
         GamepadButton::rs,
         GamepadButton::lt,
         GamepadButton::rt,
         GamepadButton::lb,
         GamepadButton::rb,
         GamepadButton::d_pad_down,
         GamepadButton::d_pad_up,
         GamepadButton::d_pad_left,
         GamepadButton::d_pad_right,
      };
      widget->addItem(tr("None"), (int)GamepadButton::none);
      for (const auto b : buttons) {
         widget->addItem(nameOf(b), (int)b);
      }
      
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget]() {
         this->setButton(Button{
            .gamepad = (GamepadButton)widget->currentData().toInt(),
         });
      });
   }
}

DKWorldinputButtonPickDialog::Button DKWorldinputButtonPickDialog::button() const {
   return this->state.value;
}
void DKWorldinputButtonPickDialog::setButton(const Button& v) {
   this->state.value = v;

   const auto blocker_a = QSignalBlocker(this->ui.mousePicker);
   const auto blocker_b = QSignalBlocker(this->ui.gamepadPicker);

   if (v.key.vk != cobb::keyboard::virtual_key::none) {
      this->ui.keyEdit->setText(nameOf(v.key.vk));
      this->ui.mousePicker->setCurrentIndex(0);
      this->ui.gamepadPicker->setCurrentIndex(0);
   } else if (v.mouse != Qt::MouseButton::NoButton) {
      this->ui.keyEdit->setText("");
      this->ui.mousePicker->setCurrentIndex(
         this->ui.mousePicker->findData((int)v.mouse)
      );
      this->ui.gamepadPicker->setCurrentIndex(0);
   } else if (v.gamepad != GamepadButton::none) {
      this->ui.keyEdit->setText("");
      this->ui.mousePicker->setCurrentIndex(0);
      this->ui.gamepadPicker->setCurrentIndex(
         this->ui.gamepadPicker->findData((int)v.gamepad)
      );
   }
}

void DKWorldinputButtonPickDialog::setGamepadAllowed(bool v) {
   if (v == this->gamepadAllowed())
      return;
   this->state.allow_gamepad = v;
   this->ui.layoutGamepad->setVisible(v);
   if (!v) {
      if (this->state.value.gamepad != GamepadButton::none)
         this->setButton({});
   }
}
void DKWorldinputButtonPickDialog::setKeyboardAllowed(bool v) {
   if (v == this->keyboardAllowed())
      return;
   this->state.allow_keyboard = v;
   this->ui.layoutKey->setVisible(v);
   if (!v) {
      if (this->state.value.key.vk != cobb::keyboard::virtual_key::none)
         this->setButton({});
   }
}
void DKWorldinputButtonPickDialog::setMouseAllowed(bool v) {
   if (v == this->mouseAllowed())
      return;
   this->state.allow_mouse = v;
   this->ui.layoutMouse->setVisible(v);
   if (!v) {
      if (this->state.value.mouse != Qt::MouseButton::NoButton)
         this->setButton({});
   }
}

/*static*/ QString DKWorldinputButtonPickDialog::nameOf(Qt::MouseButton button) {
   switch (button) {
      case Qt::MouseButton::NoButton:
         return {};

      case Qt::MouseButton::LeftButton:
         return tr("LMB", "mouse button");
      case Qt::MouseButton::RightButton:
         return tr("RMB", "mouse button");
      case Qt::MouseButton::MiddleButton:
         return tr("MMB", "mouse button");
      case Qt::MouseButton::XButton1:
         return tr("Mouse X1", "mouse button");
      case Qt::MouseButton::XButton2:
         return tr("Mouse X2", "mouse button");
      case Qt::MouseButton::ExtraButton3:
         return tr("Mouse X3", "mouse button");
      case Qt::MouseButton::ExtraButton4:
         return tr("Mouse X4", "mouse button");
      case Qt::MouseButton::ExtraButton5:
         return tr("Mouse X5", "mouse button");
      case Qt::MouseButton::ExtraButton6:
         return tr("Mouse X6", "mouse button");
      case Qt::MouseButton::ExtraButton7:
         return tr("Mouse X7", "mouse button");
      case Qt::MouseButton::ExtraButton8:
         return tr("Mouse X8", "mouse button");
      case Qt::MouseButton::ExtraButton9:
         return tr("Mouse X9", "mouse button");
      case Qt::MouseButton::ExtraButton10:
         return tr("Mouse X10", "mouse button");
      case Qt::MouseButton::ExtraButton11:
         return tr("Mouse X11", "mouse button");
      case Qt::MouseButton::ExtraButton12:
         return tr("Mouse X12", "mouse button");
      case Qt::MouseButton::ExtraButton13:
         return tr("Mouse X13", "mouse button");
      case Qt::MouseButton::ExtraButton14:
         return tr("Mouse X14", "mouse button");
      case Qt::MouseButton::ExtraButton15:
         return tr("Mouse X15", "mouse button");
      case Qt::MouseButton::ExtraButton16:
         return tr("Mouse X16", "mouse button");
      case Qt::MouseButton::ExtraButton17:
         return tr("Mouse X17", "mouse button");
      case Qt::MouseButton::ExtraButton18:
         return tr("Mouse X18", "mouse button");
      case Qt::MouseButton::ExtraButton19:
         return tr("Mouse X19", "mouse button");
      case Qt::MouseButton::ExtraButton20:
         return tr("Mouse X20", "mouse button");
      case Qt::MouseButton::ExtraButton21:
         return tr("Mouse X21", "mouse button");
      case Qt::MouseButton::ExtraButton22:
         return tr("Mouse X22", "mouse button");
      case Qt::MouseButton::ExtraButton23:
         return tr("Mouse X23", "mouse button");
      case Qt::MouseButton::ExtraButton24:
         return tr("Mouse X24", "mouse button");
   }

   return tr("Unknown Mouse Button", "mouse button");
}
/*static*/ QString DKWorldinputButtonPickDialog::nameOf(GamepadButton button) {
   switch (button) {
      case GamepadButton::none: return {};

      case GamepadButton::a: return tr("A", "gamepad button");
      case GamepadButton::b: return tr("B", "gamepad button");
      case GamepadButton::x: return tr("X", "gamepad button");
      case GamepadButton::y: return tr("Y", "gamepad button");
      case GamepadButton::start: return tr("Start", "gamepad button");
      case GamepadButton::back: return tr("Back", "gamepad button");
      case GamepadButton::lb: return tr("Left Bumper", "gamepad button");
      case GamepadButton::rb: return tr("Right Bumper", "gamepad button");
      case GamepadButton::ls: return tr("Left Stick Click", "gamepad button");
      case GamepadButton::rs: return tr("Right Stick Click", "gamepad button");
      case GamepadButton::lt: return tr("Left Trigger", "gamepad button");
      case GamepadButton::rt: return tr("Right Trigger", "gamepad button");
      case GamepadButton::d_pad_up: return tr("D-Pad Up", "gamepad button");
      case GamepadButton::d_pad_down: return tr("D-Pad Down", "gamepad button");
      case GamepadButton::d_pad_left: return tr("D-Pad Left", "gamepad button");
      case GamepadButton::d_pad_right: return tr("D-Pad Right", "gamepad button");
   }

   return tr("Unknown Gamepad Button", "gamepad button");
}
/*static*/ QString DKWorldinputButtonPickDialog::nameOf(cobb::keyboard::virtual_key vk) {
   if (vk == cobb::keyboard::virtual_key::none)
      return {};

   cobb::keyboard::key k(vk);
   k.make_complete();

   std::wstring name = k.get_key_name();
   return QString::fromStdWString(name);
}
/*static*/ QString DKWorldinputButtonPickDialog::nameOf(const Button& button) {
   auto out = nameOf(button.mouse);
   if (out.isEmpty()) {
      out = nameOf(button.gamepad);
      if (out.isEmpty()) {
         out = nameOf(button.key.vk);
         if (!out.isEmpty())
            out = "Key: " + out;
      }
   }
   return out;
}

bool DKWorldinputButtonPickDialog::eventFilter(QObject* target, QEvent* event) {
   if (target == this->ui.keyEdit) {
      if (event->type() == QEvent::Type::KeyPress) {
         auto* widget = this->ui.keyEdit;
         auto* casted = (QKeyEvent*)event;
         if (!casted->isAutoRepeat()) {
            auto vk = (cobb::keyboard::virtual_key)casted->nativeVirtualKey();
            this->setButton(Button{
               .key = vk
            });
         }
         return true; // swallow event
      }
   } else if (target == this->ui.mouseEdit) {
      auto* widget = this->ui.mouseEdit;
      switch (event->type()) {
         case QEvent::Type::MouseButtonPress:
            widget->setDown(true);
            return true; // swallow event
         case QEvent::Type::MouseButtonRelease:
            if (widget->isDown()) {
               this->setButton(Button{
                  .mouse = ((QMouseEvent*)event)->button()
               });
               return true; // swallow event
            }
            break;
      }
   }
   return false;
}