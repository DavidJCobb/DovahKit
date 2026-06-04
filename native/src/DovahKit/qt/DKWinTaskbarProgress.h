#pragma once
#include <QObject>

class DKWinTaskbarProgress : public QObject {
   Q_OBJECT;
   public:
      DKWinTaskbarProgress(QObject* parent = nullptr);

   public:
      constexpr bool isPaused() const noexcept { return this->_data.paused; }
      constexpr bool isStopped() const noexcept { return this->_data.stopped; }
      constexpr bool isVisible() const noexcept { return this->_data.visible; }
      constexpr int maximum() const noexcept { return this->_data.maximum; }
      constexpr int minimum() const noexcept { return this->_data.minimum; }
      constexpr int value() const noexcept { return this->_data.value; }

   public slots:
      void hide();
      void pause();
      void reset();
      void resume();
      void setMaximum(int);
      void setMinimum(int);
      void setPaused(bool);
      void setRange(int minimum, int maximum);
      void setValue(int);
      void setVisible(bool);
      void show();
      void stop();

   signals:
      void maximumChanged(int);
      void minimumChanged(int);
      void pausedChanged(bool);
      void stoppedChanged(bool);
      void valueChanged(int);
      void visibilityChanged(bool visible);

   protected:
      struct {
         int  value   = 0;
         int  minimum = 0;
         int  maximum = 100;
         bool visible = false;
         bool paused  = false;
         bool stopped = false;
      } _data;
};