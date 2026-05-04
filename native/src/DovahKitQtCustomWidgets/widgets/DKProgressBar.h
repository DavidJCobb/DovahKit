#pragma once
#include <QProgressBar>

class DKProgressBar : public QProgressBar {
   Q_OBJECT;
   public:
      bool isIndeterminate() const;
      bool isReset() const;

      void makeIndeterminate(); // convenience

      // reimp: if changing the range causes the value to fall out of range, clamp the value instead of resetting
      void setMinimum(int);
      void setMaximum(int);
      void setRange(int, int);

      // reimp: don't forcibly hide the text when the progress bar is indeterminate
      virtual QString text() const override;

      // reimp: fix text color when the bar is indeterminate
      virtual void paintEvent(QPaintEvent*) override;
};