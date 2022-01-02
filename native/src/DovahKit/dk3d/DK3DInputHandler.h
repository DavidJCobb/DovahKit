#pragma once
#include <array>
#include <bitset>
#include <vector>
#include <QObject>
#include "../helpers/bitfield_array.h"
#include "../helpers/passkey.h"
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "chrono.h"
#include "input_result.h"
#include "OSKeyboardState.h"
#include "XInputGamepadState.h"
#include "bind_tree/tree.h"
#include "devices/keyboard_mouse.h"
#include "devices/xinput.h"
#include "enums/axis2D.h"
#include "enums/input_device_type.h"
#include "enums/scalar_control.h"
#include "enums/vector_control.h"
#include "inputs/bound_input.h"
#include "widgets/DKVulkanView.h"

class DK3DInputHandler : public QObject {
   Q_OBJECT;
   public:
      using timestamp_t = DK3D::timestamp_t;

   protected:
      DK3DInputHandler();
      ~DK3DInputHandler();
   public:
      static DK3DInputHandler& get() {
         static DK3DInputHandler instance;
         return instance;
      }

      float   scalarControlValue(DK3D::scalar_control, DK3D::axis2D axis = DK3D::axis2D::x) const;
      QPointF vectorControlValue(DK3D::vector_control) const;

      DK3D::input_result inputResultOf(const DK3D::inputs::bound_input&) const;

   public:
      DK3D::binds::tree bindingsFor(DK3D::input_device_type) const;
      void viewFocusChange(DKVulkanView* target, bool has_focus);

   public slots:
      void setTargetView(DKVulkanView* target);
      DKVulkanCameraUpdate update(DKVulkanView* subject); // TODO: should return something else -- a more complete command list -- in the future

      void setBindingsFor(DK3D::input_device_type, const DK3D::binds::tree&);

   protected slots:
      void ignoreAllHeldKeys(timestamp_t now = DK3D::zero_timestamp);
      void updateAllKeys(timestamp_t now);

   protected:
      DK3D::devices::keyboard_mouse keyboard_state;
      DK3D::devices::xinput         gamepad_state;

      struct {
         QPointer<DKVulkanView> target_view;
         timestamp_t last_update = DK3D::zero_timestamp;
      } state;
      struct {
         DK3D::binds::tree keyboard = DK3D::binds::tree(DK3D::input_device_type::keyboard_mouse);
         DK3D::binds::tree gamepad  = DK3D::binds::tree(DK3D::input_device_type::xinput);
      } binds;

   public:
      using passkey_to_bind_tree = cobb::passkey<DK3DInputHandler, DK3D::binds::tree>;
      //
      bool _isButtonProcessed(passkey_to_bind_tree, const DK3D::inputs::button&) const;
      void _markButtonProcessed(passkey_to_bind_tree, const DK3D::inputs::button&);
};