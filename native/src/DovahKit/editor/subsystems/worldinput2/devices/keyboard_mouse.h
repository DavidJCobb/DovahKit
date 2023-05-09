#pragma once
#include <QPoint>
#include <QWidget>
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
   class keyboard_mouse final : public abstract_device_handler {
      public:
         static constexpr size_t vk_code_count = 256;

      public:
         components::buttoned_device<vk_code_count> buttons;
         struct {
            timestamp_t last_movement = zero_timestamp;
            bool is_stale = false;
            //
            QPoint pos;  // mouse screen position; tracked so we can generate the (move) field
            QPoint move; // distance the mouse moved since the last update
            //
            struct {
               QPoint pos;
               timestamp_t time = {};
            } last_click; // primary button only; used to identify double-clicks
         } mouse;
         struct {
            //
            // Fields relating to system configuration, e.g. whether the left and right mouse buttons 
            // have been swapped.
            //
            struct {
               bool has_scroll_wheel = false;
               bool swap_left_right  = false;
               uint double_click_ms  = 500;
               struct {
                  QPoint double_click; // second click must occur within this distance of the first (rectangle)
                  QPoint drag;         // minimum mousemove distance before a click becomes a drag (rectangle)
               } hitboxes;
            } mouse;
         } system;

      protected:
         bool _mouseup_handler_for_double_click(timestamp_t now);
      public:
         void recheck_mouse_metrics();

         void ignore_all_down();

         void update(timestamp_t now, const QWidget& view);
         void update_buttons(timestamp_t now);
         void update_pointer(const QWidget& view);

         virtual device_button_state get_state_of(const inputs::button&) const final override;
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