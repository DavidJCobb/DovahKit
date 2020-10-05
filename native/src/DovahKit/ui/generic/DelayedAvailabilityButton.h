#pragma once
#include <QColor>
#include <QPushButton>
#include <QTimer>

class DelayedAvailabilityButton : public QPushButton {
   Q_OBJECT
   public:
      DelayedAvailabilityButton(QWidget* parent = nullptr);
      //
      enum Style {
         Bar, // a bar is drawn overtop the bottom of the button
         Spinner, // a circular progress meter is drawn on the button's left side (note: wide text labels may be covered)
      };
      //
      int delay() const noexcept;
      void setDelay(int ms);
      void restartDelay();
      //
      inline Style style() const noexcept { return this->_style; }
      inline void setStyle(Style s) { this->_style = s; }
      
      // The progress bar appears to be this many milliseconds ahead of where it actually is; this allows the 
      // user to be able to see it fill up just before the button enables.
      inline int completionTimeBuffer() const noexcept { return this->_completionTimeBuffer; }
      void setCompletionTimeBuffer(int ms) { this->_completionTimeBuffer = ms; }
      
      inline bool spinnerFillCoversBorder() const noexcept { return this->_spinnerConfig.fillCoversBorder; }
      void setSpinnerFillCoversBorder(bool b) { this->_spinnerConfig.fillCoversBorder = b; }
      
      inline QColor spinnerBorderColor() const noexcept { return this->_spinnerConfig.border; }
      inline QColor spinnerBackgroundColor() const noexcept { return this->_spinnerConfig.background; }
      inline QColor spinnerFillColor() const noexcept { return this->_spinnerConfig.fill; }
      inline void setSpinnerBorderColor(const QColor& c) { this->_spinnerConfig.border = c; }
      inline void setSpinnerBackgroundColor(const QColor& c) { this->_spinnerConfig.background = c; }
      inline void setSpinnerFillColor(const QColor& c) { this->_spinnerConfig.fill = c; }
      //
      void setDisabled(bool);
      void setEnabled(bool);
      //
   protected:
      QTimer _timer;
      QTimer _paintTimer;
      Style  _style = Style::Spinner;
      int    _completionTimeBuffer = 100;
      struct {
         QColor border;
         QColor background;
         QColor fill;
         bool   fillCoversBorder = true;
      } _spinnerConfig;
      //
      void paintEvent(QPaintEvent* event);
};