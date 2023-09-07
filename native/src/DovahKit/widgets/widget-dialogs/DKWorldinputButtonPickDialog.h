#pragma once
#include "ui_DKWorldinputButtonPickDialog.h"
#include <QDialog>
#include <QString>
#include "helpers/keyboard/virtual_key.h"
#include "editor/subsystems/worldinput/inputs/button.h"

class QComboBox;
class QLineEdit;
class QPushButton;

class DKWorldinputButtonPickDialog : public QDialog {
   Q_OBJECT;
   public:
      using Button = dovahkit::subsystems::worldinput::inputs::button;
      using GamepadButton = decltype(Button::gamepad);

   public:
      DKWorldinputButtonPickDialog(QWidget* parent = nullptr);

      Button button() const;
      void setButton(const Button&);

      constexpr bool gamepadAllowed() const noexcept { return this->state.allow_gamepad; }
      constexpr bool keyboardAllowed() const noexcept { return this->state.allow_keyboard; }
      constexpr bool mouseAllowed() const noexcept { return this->state.allow_mouse; }
      void setGamepadAllowed(bool);
      void setKeyboardAllowed(bool);
      void setMouseAllowed(bool);

      static QString nameOf(Qt::MouseButton);
      static QString nameOf(GamepadButton);
      static QString nameOf(cobb::keyboard::virtual_key);
      static QString nameOf(const Button&);

   protected:
      Ui::DKWorldinputButtonPickDialog ui;
      struct {
         bool allow_gamepad  = true;
         bool allow_keyboard = true;
         bool allow_mouse    = true;
         Button value;
      } state;

      virtual bool eventFilter(QObject* target, QEvent* event) override;
};