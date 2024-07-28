#pragma once
#include <QSlider>
#include <QWidget>

class DKFloatSlider : public QWidget {
   Q_OBJECT;
   public:
      DKFloatSlider(QWidget* parent = nullptr);
      DKFloatSlider(Qt::Orientation, QWidget* parent = nullptr);

      constexpr unsigned int decimals() const noexcept { return this->_properties.decimals; }
      constexpr float maximum() const noexcept { return this->_properties.range.max; }
      constexpr float minimum() const noexcept { return this->_properties.range.min; }

      bool hasTracking() const;
      void setTracking(bool);

      void setDecimals(unsigned int);
      void setMaximum(float);
      void setMinimum(float);

      constexpr float tickInterval() const noexcept { return this->_properties.tick_interval; }
      void setTickInterval(float);

      QSlider::TickPosition tickPosition();
      void setTickPosition(QSlider::TickPosition);

      float value() const;
      void setValue(float);

      bool invertedAppearance() const;
      bool invertedControls() const;
      Qt::Orientation orientation() const;
      //
      void setInvertedAppearance(bool);
      void setInvertedControls(bool);
      void setOrientation(Qt::Orientation);

      bool isSliderDown();

   signals:
      void valueChanged(float);

   protected:
      QSlider* _slider = nullptr; // for display
      struct {
         unsigned int decimals = 0;
         struct {
            float min = 0;
            float max = 100;
         } range;
         float tick_interval = 0;
      } _properties;
      struct {
         float scale = 1; // value times this equals QSlider value
      } _state;

      void _recalculate_slider_units();
};
