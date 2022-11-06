#pragma once
#include <cstdint>
#include <QPoint>
#include "./enums/button.h"

namespace dovahkit::subsystems::xinput {
   struct gamepad {
      struct {
         QPointF ls = {};
         QPointF rs = {};
      } raw;
      QPointF  ls = {};
      QPointF  rs = {};
      float    lt = 0;
      float    rt = 0;
      uint16_t buttons = 0;
      
      bool is_button_down(button b) const noexcept;
   };
}