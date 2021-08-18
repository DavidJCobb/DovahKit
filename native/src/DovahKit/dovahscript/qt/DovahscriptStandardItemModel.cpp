#include "DovahscriptStandardItemModel.h"
#include <QWidget>

void DovahscriptStandardItemModel::associatedWidgetDestroyed(QObject* widget) {
   assert(widget->isWidgetType());
   this->dissociateFromWidget((QWidget*)widget);
}

QList<QWidget*> DovahscriptStandardItemModel::associatedWidgets() {
   return this->_associated_widgets;
}
void DovahscriptStandardItemModel::associateWithWidget(QWidget* widget) {
   auto& list = this->_associated_widgets;
   if (list.contains(widget))
      return;
   QObject::connect(widget, &QObject::destroyed, this, &DovahscriptStandardItemModel::associatedWidgetDestroyed);
   list.push_back(widget);
}
void DovahscriptStandardItemModel::dissociateFromWidget(QWidget* widget) {
   this->_associated_widgets.removeOne(widget);
   if (this->_associated_widgets.isEmpty())
      emit this->dissociatedFromAll();
}