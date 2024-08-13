#pragma once
#include <QTabBar>

// FIX: Using the scroll wheel on the tabbar should not allow you to scroll into 
//      hidden tabs.
class DKTabBar : public QTabBar {
   Q_OBJECT;
   public:
      using QTabBar::QTabBar;

   protected:
      virtual void keyPressEvent(QKeyEvent*) override;
      #if QT_CONFIG(wheelevent)
         virtual void wheelEvent(QWheelEvent* event) override;
      #endif

      // The sign of `dir` indicates whether to move to the left or right.
      // Selects the next focusable (visible and enabled) tab in that direction, if any.
      void _moveFocusInDirection(int dir);
};
