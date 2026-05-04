#include "./DKStatusBar.h"
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

DKStatusBar::DKStatusBar(QWidget* parent) : QStatusBar(parent) {
   {
      //
      // Win32 FlashWindowEx uses the caret blink rate by default, so we'll do that too.
      //
      float blink = QApplication::cursorFlashTime();
      if (blink > 0) {
         blink /= 1000; // from ms to s
         blink /= 2;    // cursor flash time is the time to disappear and reappear, so halve it for the...
         this->_flash.duration = blink; // ...appear time
         this->_flash.interval = blink; // ...disappear time
      }
   }
   this->_flash.color = QColor(255, 128, 0);
   this->_flash.updater.setInterval(1000 / 30);
   this->_flash.updater.setSingleShot(false);
   QObject::connect(&this->_flash.updater, &QTimer::timeout, this, &DKStatusBar::_on_flash_update);
}

void DKStatusBar::flash(QWidget* widget) {
   if (!this->_contains_widget(widget))
      return;

   float time = (this->_flash.duration + this->_flash.interval) * this->_flash.count;

   for (auto* flash : this->_state.flashes) {
      if (flash->target == widget) {
         /*//
         flash->timer.stop();
         flash->timer.setInterval(time);
         flash->timer.start();
         //*/
         return;
      }
   }
   auto* item = new FlashState;
   this->_state.flashes.push_back(item);
   item->target = widget;
   auto& timer = item->timer;
   timer.setInterval(time * 1000);
   timer.setSingleShot(true);
   timer.start();

   if (!this->_flash.updater.isActive())
      this->_flash.updater.start();
}

#pragma region Properties
   void DKStatusBar::setFlashColor(QColor v) {
      auto& dst = this->_flash.color;
      if (dst == v)
         return;
      dst = v;
      if (this->_flash.updater.isActive())
         this->update();
   }
   void DKStatusBar::setFlashCount(size_t v) {
      auto& dst = this->_flash.count;
      if (dst == v)
         return;
      dst = v;
   }
   void DKStatusBar::setFlashDuration(float v) {
      auto& dst = this->_flash.duration;
      if (dst == v)
         return;
      dst = v;
   }
   void DKStatusBar::setFlashInterval(float v) {
      auto& dst = this->_flash.interval;
      if (dst == v)
         return;
      dst = v;
   }
#pragma endregion

bool DKStatusBar::_contains_widget(const QWidget* widget) const {
   //
   // NOTE: Don't just check for direct-child relationships. QStatusBar creates 
   //       nested layouts, and that nesting can vary depending on which native 
   //       widgets (e.g. a resize handle) are also shown.
   //
   const QObject* object = widget;
   while (object = object->parent()) {
      if (!object)
         break;
      if (object == this)
         return true;
   }
   return false;
}
void DKStatusBar::_on_flash_update() {
   this->update();
   this->_remove_dead_flashes();
   if (this->_state.flashes.empty()) {
      this->_flash.updater.stop();
   }
}
void DKStatusBar::_remove_dead_flashes() {
   for (auto*& flash : this->_state.flashes) {
      if (!flash->target || !flash->timer.isActive() || !this->_contains_widget(flash->target)) {
         delete flash;
         flash = nullptr;
      }
   }
   std::erase_if(
      this->_state.flashes,
      [this](const FlashState* dst) {
         return dst == nullptr;
      }
   );
}

/*virtual*/ void DKStatusBar::paintEvent(QPaintEvent* event) /*override*/ {
   if (!this->_state.flashes.empty()) {
      QPainter painter(this);

      auto er = event->rect();
      for (auto* flash : this->_state.flashes) {
         auto* widget = flash->target.data();
         if (!widget || !widget->isVisible())
            continue;

         auto rect = widget->geometry();
         //
         // The frame for status bar items is the item geometry with a hardcoded outward 
         // offset of 2px on the horizontal and 1px on the vertical.
         //
         rect.adjust(-2, -1, 2, 1);
         // 
         // The QStatusBar's internal layout also has a spacing of 6px between items, but 
         // the frame is drawn into this area. On Windows, in practice, the frame is offset 
         // 1px to the left.
         //
         rect.adjust(-3, 0, 0, 0);

         if (!er.intersects(rect))
            continue;

         auto color = this->_flash.color;
         {
            float single  = this->_flash.duration + this->_flash.interval;
            float elapsed = (float)flash->timer.remainingTime() / 1000.0F;

            float a = elapsed / single;
            float b = this->_flash.duration / single;
            a -= (unsigned int)a;

            if (a < b) {
               if (a < 0) // floating-point imprecision
                  a = 0;
               color.setAlphaF(1.0F - a);
            } else {
               color.setAlpha(0);
            }
         }
         painter.fillRect(rect, color);
      }
   }
   QStatusBar::paintEvent(event);
}