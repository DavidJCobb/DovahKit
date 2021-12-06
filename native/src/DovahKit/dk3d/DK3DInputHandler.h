#pragma once
#include <array>
#include <bitset>
#include <vector>
#include <QObject>
#include "../helpers/bitfield_array.h"
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "chrono.h"
#include "Binding.h"
#include "BoundInput.h"
#include "InputResult.h"
#include "OSKeyboardState.h"
#include "XInputGamepadState.h"
#include "enums/InputDevice.h"
#include "widgets/DKVulkanView.h"

class DK3DInputHandler : public QObject {
   Q_OBJECT;
   public:
      using Axis2D        = DK3D::Axis2D;
      using Binding       = DK3D::Binding;
      using BoundInput    = DK3D::BoundInput;
      using InputResult   = DK3D::InputResult;
      using ScalarControl = DK3D::ScalarControl;
      using VectorControl = DK3D::VectorControl;
      using timestamp_t   = DK3D::timestamp_t;

   protected:
      DK3DInputHandler();
      ~DK3DInputHandler();
   public:
      static DK3DInputHandler& get() {
         static DK3DInputHandler instance;
         return instance;
      }

      float   scalarControlValue(ScalarControl, Axis2D axis = Axis2D::X) const;
      QPointF vectorControlValue(VectorControl) const;

      InputResult inputResultOf(const BoundInput&) const;

   public:
      QVector<Binding> bindingsFor(DK3D::InputDevice) const;
      void viewFocusChange(DKVulkanView* target, bool has_focus);

   public slots:
      void setTargetView(DKVulkanView* target);
      DKVulkanCameraUpdate update(DKVulkanView* subject); // TODO: should return something else -- a more complete command list -- in the future

      void debugOpenBindEditWindow(); // opens a window for testing, to edit keybinds; we'll be able to use the normal UI once bindings are less hardcoded
      void setBindingsFor(DK3D::InputDevice, const QVector<Binding>&);

   protected slots:
      void ignoreAllHeldKeys(timestamp_t now = DK3D::zero_timestamp);
      void updateAllKeys(timestamp_t now);

   protected:
      DK3D::OSKeyboardState    keyboard_state;
      DK3D::XInputGamepadState gamepad_state;

      struct {
         QPointer<DKVulkanView> target_view;
         timestamp_t last_update = DK3D::zero_timestamp;
      } state;
      struct {
         QVector<Binding> keyboard;
         QVector<Binding> gamepad;
      } binds;
};