#pragma once
#include <Qt>
#include <QWidget>
#include "editor/subsystems/xinput/enums/button.h"
#include "editor/subsystems/xinput/gamepad.h"
#include "../enums/button_press_type.h"
#include "../chrono.h"
#include "../device_button_state.h"

#include "./components/buttoned_device.h"
#include "./abstract_device_handler.h"

namespace dovahkit::subsystems::worldinput2 {
   namespace inputs {
      struct button;
   }
}

namespace dovahkit::subsystems::worldinput2::devices {
   class xinput final : public abstract_device_handler {
      public:
         using button = subsystems::xinput::button;
         static constexpr size_t button_count = 16;

      public:
         bool is_connected = false;
         components::buttoned_device<button_count> buttons;
         struct {
            QPointF ls = {};
            QPointF rs = {};
         } vectors;
         struct {
            float lt = 0;
            float rt = 0;
         } scalars;

         void ignore_all_down();

         void update(timestamp_t now, const QWidget& view, bool connected, const subsystems::xinput::gamepad&);
         void update_buttons(timestamp_t now, bool connected, const subsystems::xinput::gamepad&);
         void update_pointer(const QWidget& view);

         virtual device_button_state get_state_of(const inputs::button&) const final;
         virtual const device_button_claim& get_existing_claim_of(const inputs::button&) const final override;
         virtual device_button_claim& get_pending_claim_of(const inputs::button&) final override;
         virtual const device_button_claim& get_pending_claim_of(const inputs::button&) const final override;
         //
         virtual bool button_is_valid(const inputs::button&) const override final;
         //
         virtual range_control_state get_range_control_state(scalar_input_control, axis2D) const override final;
         virtual range_control_state get_range_control_state(vector_input_control) const override final;
         virtual QPointF get_range_control_value(scalar_input_control, axis2D) const override final;
         virtual QPointF get_range_control_value(vector_input_control) const override final;
         //
      protected:
         virtual interruption_check _prepare_interruption_check_impl() const override final;
   };
}