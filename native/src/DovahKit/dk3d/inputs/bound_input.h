#pragma once
#include "button.h"
#include "dk3d/enums/axis2D.h"
#include "dk3d/enums/scalar_control.h"
#include "dk3d/enums/vector_control.h"

namespace DK3D::inputs {
   struct bound_input {
      button button;
      struct {
         scalar_control input = scalar_control::none;
         axis2D axis = axis2D::x;
      } scalar;
      struct {
         vector_control input = vector_control::none;
      } vector;

      inline bool is_button() const noexcept {
         return !this->button.empty();
      }
      inline bool is_scalar() const noexcept {
         return !is_button() && this->scalar.input != scalar_control::none;
      }
      inline bool is_vector() const noexcept {
         return !is_button() && !is_scalar() && this->vector.input != vector_control::none;
      }

      static bound_input from_key(QChar, button_press_type mod = button_press_type::tap);
      static bound_input from_key(Qt::Key, button_press_type mod = button_press_type::tap);
      static bound_input from_mouse_button(Qt::MouseButton, button_press_type mod = button_press_type::tap);
      static bound_input from_xinput_button(xinput_button, button_press_type mod = button_press_type::tap);
   };
}

#include <QObject>
Q_DECLARE_METATYPE(DK3D::inputs::bound_input)