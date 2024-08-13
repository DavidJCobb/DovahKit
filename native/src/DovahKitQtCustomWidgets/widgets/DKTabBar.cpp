#include "./DKTabBar.h"
#include <cstdlib> // abs
#include <QKeyEvent>
#if QT_CONFIG(wheelevent)
   #include <QWheelEvent>
#endif

/*virtual*/ void DKTabBar::keyPressEvent(QKeyEvent* event) /*override*/ {
   int offset;
   switch (event->key()) {
      case Qt::Key_Left:
         offset = -1;
         break;
      case Qt::Key_Right:
         offset = +1;
         break;
      default:
         event->ignore();
         return;
   }
   if (this->isRightToLeft())
      offset = -offset;
   this->_moveFocusInDirection(offset);
}
#if QT_CONFIG(wheelevent)
   /*virtual*/ void DKTabBar::wheelEvent(QWheelEvent* event) /*override*/ {
      #ifndef Q_OS_MAC
         int delta;
         {
            auto point = event->angleDelta();
            auto x     = point.x();
            auto y     = point.y();
            if (abs(x) > abs(y))
               delta = x;
            else
               delta = y;
         }
         int offset = delta > 0 ? -1 : 1;
         this->_moveFocusInDirection(offset);
         QWidget::wheelEvent(event);
      #endif
   }
#endif

void DKTabBar::_moveFocusInDirection(int offset) {
   auto current = this->currentIndex();
   auto count   = this->count();
   for (int index = current + offset; index >= 0 && index < count; index += offset) {
      if (!this->isTabVisible(index) || !this->isTabEnabled(index))
         continue;
      this->setCurrentIndex(index);
      break;
   }
}