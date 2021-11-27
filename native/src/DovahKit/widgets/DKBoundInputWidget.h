#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "DKKeyPickerWidget.h"
#if !defined(QT_DESIGNER_LIB)
   #include "../dk3d/BoundInput.h"
#else
   namespace DK3D {
      enum class ControlType {
         Boolean,
         Scalar,
         Vector,
      };
      struct BoundInput {};
   }
#endif

class DKBoundInputWidget : public QWidget {
   Q_OBJECT;
   public:
      using ControlType = DK3D::ControlType;
      enum class InputDevice {
         KeyboardMouse,
         XInput,
      };

   public:
      DKBoundInputWidget(QWidget* parent = nullptr);

      inline InputDevice inputDevice() const noexcept { return this->state.inputDevice; }
      DK3D::BoundInput value() const;

   public slots:
      void setInputDevice(InputDevice);
      void setValue(const DK3D::BoundInput&);

   signals:

   protected:
      struct {
         InputDevice inputDevice = InputDevice::KeyboardMouse;
      } state;
      struct {
         QComboBox* control_type = nullptr;
         struct {
            QLabel* label = nullptr;
            DKKeyPickerWidget* control = nullptr;
            QComboBox* xinput = nullptr;
            QComboBox* mod = nullptr;
         } boolean;
         struct {
            QLabel* label = nullptr;
            QComboBox* control = nullptr;
            QComboBox* sign    = nullptr;
         } scalar;
         struct {
            QLabel* label = nullptr;
            QComboBox* control = nullptr;
         } vector;
      } subwidgets;
      
   protected slots:
      void _onControlTypeChange();
      void _onInputDeviceChange();
};