#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "DKKeyPickerWidget.h"
#if !defined(QT_DESIGNER_LIB)
   #include "editor/subsystems/worldinput/enums/control_type.h"
   #include "editor/subsystems/worldinput/enums/input_device_type.h"
   #include "editor/subsystems/worldinput/inputs/bound_input.h"
#else
   namespace dovahkit::subsystems::worldinput {
      enum class control_type {
         button,
         scalar,
         vector,
      };
      enum class input_device_type {
         keyboard_mouse,
         xinput,
      };
      namespace inputs {
         struct bound_input {};
      }
   }
#endif

class DKBoundInputWidget : public QWidget {
   Q_OBJECT;
   public:
      DKBoundInputWidget(QWidget* parent = nullptr);

      using bound_input       = dovahkit::subsystems::worldinput::inputs::bound_input;
      using input_device_type = dovahkit::subsystems::worldinput::input_device_type;

      inline input_device_type inputDevice() const noexcept { return this->state.inputDevice; }
      bound_input value() const;

   public slots:
      void setInputDevice(input_device_type);
      void setValue(const bound_input&);

   signals:
      void valueChanged(const bound_input&);

   protected:
      static constexpr auto ControlRole = Qt::ItemDataRole::UserRole;
      static constexpr auto AxisRole    = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1);

      struct {
         input_device_type inputDevice = input_device_type::keyboard_mouse;
      } state;
      struct {
         QComboBox* control_type = nullptr;
         struct {
            QLabel*  label   = nullptr;
            QWidget* wrapper = nullptr;
            //
            DKKeyPickerWidget* control = nullptr;
            QComboBox* xinput = nullptr;
            QComboBox* mod = nullptr;
         } boolean;
         struct {
            QLabel*  label   = nullptr;
            QWidget* wrapper = nullptr;
            //
            QComboBox* control = nullptr;
            QComboBox* sign    = nullptr;
         } scalar;
         struct {
            QLabel*  label   = nullptr;
            QWidget* wrapper = nullptr;
            //
            QComboBox* control = nullptr;
         } vector;
      } subwidgets;
      
   protected slots:
      void _onControlTypeChange();
      void _onInputDeviceChange();
      void _sendValueChanged();
};