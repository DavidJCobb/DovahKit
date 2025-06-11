#pragma once
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include "widgets/DKFloatSlider.h"

namespace ui {
   //
   // Pair a slider to a spinbox. The spinbox is considered the "canonical" 
   // of the two widgets: if you want to set the value from outside, e.g. 
   // using `ui::bind`, then do so on the spinbox.
   // 
   // We really should just make a custom widget that pairs a slider and a 
   // spinbox together, but for now, this is a cheap enough hack.
   //
   extern void pair_slider_to_spinbox(QSlider*, QSpinBox*);
   extern void pair_slider_to_spinbox(DKFloatSlider*, QDoubleSpinBox*);
}