#include "./DKTabWidget.h"
#include "./DKTabBar.h"

DKTabWidget::DKTabWidget(QWidget* parent) : QTabWidget(parent) {
   this->setTabBar(new DKTabBar(this));
}

void DKTabWidget::setTabBar(QTabBar* after) {
   //
   // When you replace a QTabWidget's tabbar, the widget does not properly 
   // update properties on the new QTabBar to match properties on the widget. 
   // It only does those updates when it initially creates its tabbar, and 
   // when those properties change (so `setTabPosition(tabPosition())`, for 
   // example, is not a fix).
   //
   bool can_preserve_properties = true;
   struct {
      bool           draw_base;
      QTabBar::Shape shape;
   } preserved;

   QTabBar::Shape shape = {};
   if (auto* prior = this->tabBar()) {
      can_preserve_properties = true;
      //
      preserved.draw_base = prior->drawBase();
      preserved.shape     = prior->shape();
   }

   QTabWidget::setTabBar(after);

   if (can_preserve_properties) {
      if (after) {
         after->setDrawBase(preserved.draw_base);
         after->setShape(preserved.shape);
      }
   }
}