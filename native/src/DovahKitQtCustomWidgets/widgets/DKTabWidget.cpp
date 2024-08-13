#include "./DKTabWidget.h"
#include "./DKTabBar.h"

DKTabWidget::DKTabWidget(QWidget* parent) : QTabWidget(parent) {
   this->setTabBar(new DKTabBar(this));
}