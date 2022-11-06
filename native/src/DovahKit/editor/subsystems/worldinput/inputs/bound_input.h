#pragma once
#include "button.h"
#include "../enums/axis2D.h"
#include "../enums/control_type.h"
#include "../enums/scalar_control.h"
#include "../enums/vector_control.h"

namespace dovahkit::subsystems::worldinput::inputs {
   struct bound_input {
      button button;
      struct {
         scalar_control input = scalar_control::none;
         axis2D axis = axis2D::x;
      } scalar;
      struct {
         vector_control input = vector_control::none;
      } vector;

      control_type get_control_type() const noexcept {
         if (!this->button.empty())
            return control_type::button;
         if (this->scalar.input != scalar_control::none)
            return control_type::scalar;
         if (this->vector.input != vector_control::none)
            return control_type::vector;
         return control_type::none;
      }

      inline bool is_button() const noexcept {
         return get_control_type() == control_type::button;
      }
      inline bool is_scalar() const noexcept {
         return get_control_type() == control_type::scalar;
      }
      inline bool is_vector() const noexcept {
         return get_control_type() == control_type::vector;
      }

      static bound_input from_key(QChar, button_press_type mod = button_press_type::tap);
      static bound_input from_key(Qt::Key, button_press_type mod = button_press_type::tap);
      static bound_input from_mouse_button(Qt::MouseButton, button_press_type mod = button_press_type::tap);
      static bound_input from_xinput_button(xinput_button, button_press_type mod = button_press_type::tap);
   };
}

#include <QObject>
Q_DECLARE_METATYPE(dovahkit::subsystems::worldinput::inputs::bound_input)