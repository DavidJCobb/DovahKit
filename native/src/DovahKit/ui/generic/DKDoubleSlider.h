#pragma once
#include <QSlider>
#include <QWidget>

// UNFINISHED

class QStyleOptionSlider;

class DKDoubleSlider : public QWidget {
   Q_OBJECT;
   public:
      DKDoubleSlider(QWidget* parent = nullptr);

      using TickPosition = QSlider::TickPosition;

   protected:
      struct {
         int    decimals     = 2;
         double minimum      =   0.0;
         double maximum      = 100.0;
         double pageStep     =  10.0;
         double singleStep   =   1.0;
         double tickInterval =   0.0;
         bool   tracking     =  true;
         double value        =   0.0;
         //
         Qt::Orientation orientation  = Qt::Orientation::Horizontal;
         TickPosition    tickPosition = TickPosition::NoTicks;
      } state;

      virtual bool event(QEvent* e) override;
      virtual void paintEvent(QPaintEvent*) override;
      //
      virtual void changeEvent(QEvent* ev) override;
      virtual void keyPressEvent(QKeyEvent* ev) override;
      virtual void timerEvent(QTimerEvent* e) override;
      virtual void wheelEvent(QWheelEvent* e) override;

      void _setUpStyleOptions(QStyleOptionSlider&) const noexcept;

   public:
      inline bool hasTracking() const noexcept { return this->state.tracking; }
      inline double maximum() const noexcept { return this->state.maximum; }
      inline double minimum() const noexcept { return this->state.minimum; }
      inline double pageStep() const noexcept { return this->state.pageStep; }
      inline double singleStep() const noexcept { return this->state.singleStep; }
      inline double tickInterval() const noexcept { return this->state.tickInterval; }
      inline TickPosition tickPosition() const noexcept { return this->state.tickPosition; }
      inline double value() const noexcept { return this->state.value; }

      void setMaximum(double);
      void setMinimum(double);
      void setPageStep(double);
      void setSingleStep(double);
      void setTickInterval(double);
      void setTickPosition(TickPosition);

   public slots:
      void setRange(double min, double max);
      void setValue(double);

   signals:
      void actionTriggered(int action);
      void rangeChanged(double min, double max);
      void sliderMoved(double value);
      void sliderPressed();
      void sliderReleased();
      void valueChanged(double value);
};
