#include "DKYesNoUnsetWidgetTaskMenuExtension.h"
#include <QAction>
#include <QDesignerFormWindowInterface>

DKYesNoUnsetWidgetTaskMenuExtension::DKYesNoUnsetWidgetTaskMenuExtension(DKYesNoUnsetWidget* widget, QObject* parent) : QObject(parent), widget(widget) {
   {
      auto* action = this->actions.editText = new QAction(tr("Change text..."), this);
      QObject::connect(action, &QAction::triggered, this, &DKYesNoUnsetWidgetTaskMenuExtension::editText);
   }
}

QAction* DKYesNoUnsetWidgetTaskMenuExtension::preferredEditAction() const {
   return nullptr;
}
QList<QAction*> DKYesNoUnsetWidgetTaskMenuExtension::taskActions() const {
   return QList<QAction*>{
      this->actions.editText,
   };
}

// action responses:

void DKYesNoUnsetWidgetTaskMenuExtension::editText() {
   auto* fw = QDesignerFormWindowInterface::findFormWindow(this->widget);
   if (auto* prior = this->_last_in_place_editor.data()) {
      if (prior->targetWidget() == this->widget && prior->targetProperty() == "text") {
         prior->setFocus();
         return;
      }
      prior->forceFinishEditing();
   }
   this->_last_in_place_editor = new InPlaceTextEditor(this->widget, "text");
}