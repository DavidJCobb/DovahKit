#pragma once
#include <array>
#include <bitset>
#include <vector>
#include <QObject>
#include <QPointer>
#include "helpers/bitfield_array.h"
#include "helpers/passkey.h"
#include "./chrono.h"
#include "./input_result.h"
#include "./bind_tree/tree.h"
#include "./devices/keyboard_mouse.h"
#include "./devices/xinput.h"
#include "./enums/axis2D.h"
#include "./enums/input_device_type.h"
#include "./enums/scalar_control.h"
#include "./enums/vector_control.h"
#include "./inputs/bound_input.h"
#include "vulkan/data/DKVulkanCameraUpdate.h"
#include "widgets/DKVulkanView.h"

namespace dovahkit::subsystems::worldinput {
   class combined_tool_results;
   
   class core : public QObject {
      Q_OBJECT;
      protected:
         core();
         ~core();
      public:
         static core& get() {
            static core instance;
            return instance;
         }

         float   scalarControlValue(scalar_control, axis2D axis = axis2D::x) const;
         QPointF vectorControlValue(vector_control) const;

         input_result inputResultOf(const inputs::bound_input&) const;

      public:
         binds::tree bindingsFor(input_device_type) const;
         void viewFocusChange(DKVulkanView* target, bool has_focus);

         void update(combined_tool_results&, double& elapsed_time);

      public slots:
         void setTargetView(DKVulkanView* target);
         DKVulkanCameraUpdate update(DKVulkanView* subject); // TODO: should return something else -- a more complete command list -- in the future

         void setBindingsFor(input_device_type, const binds::tree&);

      protected slots:
         void ignoreAllHeldKeys(timestamp_t now = zero_timestamp);
         void updateAllKeys(timestamp_t now);

      protected:
         devices::keyboard_mouse keyboard_state;
         devices::xinput         gamepad_state;

         struct {
            QPointer<DKVulkanView> target_view;
            timestamp_t last_update = zero_timestamp;
         } state;
         struct {
            binds::tree keyboard = binds::tree(input_device_type::keyboard_mouse);
            binds::tree gamepad  = binds::tree(input_device_type::xinput);
         } binds;

      public:
         using passkey_to_bind_tree = cobb::passkey<core, binds::tree>;
         //
         bool _isButtonProcessed(passkey_to_bind_tree, const inputs::button&) const;
         void _markButtonProcessed(passkey_to_bind_tree, const inputs::button&);
   };
}
