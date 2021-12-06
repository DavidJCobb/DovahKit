#include "DK3DBindingOptionsContainerWidget.h"

void DK3DBindingOptionsContainerWidget::clear() {
   if (auto* layout = this->layout()) {
      //
      // When a widget is destroyed, it also destroys its layout and any items or 
      // widgets owned by the layout.
      //
      QWidget().setLayout(layout);
   }
   for (auto* widget : this->findChildren<QWidget*>()) {
      if (widget->parent() == this) {
         widget->setParent(nullptr);
         widget->deleteLater();
      }
   }
}