#pragma once
#include <QColor>
#include <QPushButton>
#include <QWidget>
#include <QTimer>

// FUN FACT: Qt's MOC sucks and can't nest anything ever! :) :) :)
class DKDelayedEnableButton_SpinnerStyleOptions {
   private:
      Q_GADGET;
      Q_PROPERTY(QColor border     MEMBER border DESIGNABLE true);
      Q_PROPERTY(QColor background MEMBER background DESIGNABLE true);
      Q_PROPERTY(QColor fill       MEMBER fill DESIGNABLE true);
      Q_PROPERTY(bool   fillCoversBorder MEMBER fillCoversBorder DESIGNABLE true);
      Q_PROPERTY(float  thickness MEMBER thickness DESIGNABLE true);
   public:
      QColor border;
      QColor background;
      QColor fill;
      bool   fillCoversBorder = true;
      float  thickness = 3.0F;
};
Q_DECLARE_TYPEINFO(DKDelayedEnableButton_SpinnerStyleOptions, Q_PRIMITIVE_TYPE);

class DKDelayedEnableButton : public QPushButton {
   Q_OBJECT;
   Q_PROPERTY(CounterStyle counterStyle READ counterStyle WRITE setCounterStyle DESIGNABLE true);
   Q_PROPERTY(int          delay        READ delay WRITE setDelay DESIGNABLE true);
   Q_PROPERTY(int          completionTimeBuffer READ completionTimeBuffer WRITE setCompletionTimeBuffer DESIGNABLE true);
   Q_PROPERTY(DKDelayedEnableButton_SpinnerStyleOptions spinnerStyleOptions READ spinnerStyleOptions WRITE setSpinnerStyleOptions DESIGNABLE true);
   public:
      enum class CounterStyle {
         Bar,     // A bar is drawn overtop the bottom edge of the button.
         Spinner, // A circular progress meter is drawn on the button's left side (note: wide text labels may be covered)
         Number,  // The text label has the number of whole seconds appended (e.g. "Foo (3)")
      };
      Q_ENUM(CounterStyle);

      using SpinnerStyleOptions = DKDelayedEnableButton_SpinnerStyleOptions;

   public:
      DKDelayedEnableButton(QWidget* parent = nullptr);

      float progress() const noexcept;
      unsigned int secondsRemaining() const noexcept;

      int delay() const noexcept;

   public slots:
      void setDelay(int ms);
      void restartDelay();
      void queueEnable();

   public:
      constexpr CounterStyle counterStyle() const noexcept { return this->_style; }
      void setCounterStyle(CounterStyle s);
      
      // The progress bar appears to be this many milliseconds ahead of where it actually is; this allows the 
      // user to be able to see it fill up just before the button enables.
      constexpr int completionTimeBuffer() const noexcept { return this->_completionTimeBuffer; }
      void setCompletionTimeBuffer(int ms) { this->_completionTimeBuffer = ms; }

      constexpr SpinnerStyleOptions spinnerStyleOptions() const noexcept { return this->_spinner_options; }
      void setSpinnerStyleOptions(const SpinnerStyleOptions& v) {
         this->_spinner_options = v;
      }
      
   protected:
      CounterStyle _style = CounterStyle::Spinner;
      int          _completionTimeBuffer = 100;
      SpinnerStyleOptions _spinner_options;
      struct {
         QTimer enable;
         QTimer repaint;
      } _timers;

   public:
      virtual QSize sizeHint() const override;

   protected:
      virtual void paintEvent(QPaintEvent* event) override;
      virtual void changeEvent(QEvent* event) override;

      void _draw_bar(QPainter&, float progress);
      void _draw_spinner(QPainter&, float progress, QRectF spinner);
      QString _effective_label() const;
};