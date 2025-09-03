#pragma once
#include <QDoubleSpinBox>
#include <QSlider>

class WeatherIntpackedFloatEditor : public QObject {
   public:
      void abandonWidgets();
      bool hasWidgets() const;
      void setWidgets(QSlider&, QDoubleSpinBox&);
      void bindTo(uint8_t&);

      constexpr float minimum() const noexcept { return this->_minimum; }
      constexpr float maximum() const noexcept { return this->_maximum; }

      void setMinimum(float);
      void setMaximum(float);
      void setRange(float, float);
      
      float byteToFloat(uint8_t) const;
      uint8_t floatToByte(float) const;

   protected:
      QSlider*        slider  = nullptr;
      QDoubleSpinBox* spinbox = nullptr;
      float _minimum = 0;
      float _maximum = 1;

      void _on_range_changed();
};