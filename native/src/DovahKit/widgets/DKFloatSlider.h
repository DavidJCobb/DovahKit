#pragma once
#include <QSlider>
#include <QWidget>

class DKFloatSlider : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation);
   Q_PROPERTY(int decimals READ decimals WRITE setDecimals);
   #if !defined(QT_PLUGIN)
      Q_PROPERTY(float value READ value WRITE setValue NOTIFY valueChanged USER true);
      Q_PROPERTY(float minimum READ minimum WRITE setMinimum);
      Q_PROPERTY(float maximum READ maximum WRITE setMaximum);
      Q_PROPERTY(float tickInterval READ tickInterval WRITE setTickInterval);
      Q_PROPERTY(float sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderMoved);
   #else
      //
      // Imagine not supporting floats as a data type in your property-editing UI. 
      // That'd be pretty dumb, right?
      //
      Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged USER true);
      Q_PROPERTY(double minimum READ minimum WRITE setMinimum);
      Q_PROPERTY(double maximum READ maximum WRITE setMaximum);
      Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval);
      Q_PROPERTY(double sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderMoved);
   #endif
   Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking);
   Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance);
   Q_PROPERTY(bool invertedControls READ invertedControls WRITE setInvertedControls);
   Q_PROPERTY(QSlider::TickPosition tickPosition READ tickPosition WRITE setTickPosition);
   Q_PROPERTY(bool sliderDown READ isSliderDown WRITE setSliderDown DESIGNABLE false);
   public:
      using TickPosition = QSlider::TickPosition;
   public:
      DKFloatSlider(QWidget* parent = nullptr);
      DKFloatSlider(Qt::Orientation, QWidget* parent = nullptr);

      virtual QSize sizeHint() const override;
      virtual QSize minimumSizeHint() const override;

      constexpr unsigned int decimals() const noexcept { return this->_properties.decimals; }
      constexpr float maximum() const noexcept { return this->_properties.range.max; }
      constexpr float minimum() const noexcept { return this->_properties.range.min; }

      bool hasTracking() const;
      void setTracking(bool);

      void setDecimals(unsigned int);
      void setMaximum(float);
      void setMinimum(float);
      void setRange(float minimum, float maximum);

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

      bool isSliderDown() const;
      void setSliderDown(bool);

      float sliderPosition() const;
      void setSliderPosition(float);

   signals:
      void sliderMoved(float);
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

      int _float_to_int_mult() const;
      void _recalculate_slider_units();
};
