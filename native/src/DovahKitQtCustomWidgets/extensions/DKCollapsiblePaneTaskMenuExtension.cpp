#include "DKCollapsiblePaneTaskMenuExtension.h"
#include <QAction>
#include <QDesignerFormWindowInterface>

#include "../tools/action_editor/ActionEditorWindow.h"

DKCollapsiblePaneTaskMenuExtension::DKCollapsiblePaneTaskMenuExtension(DKCollapsiblePane* widget, QObject* parent) : QObject(parent), widget(widget) {
   {
      auto* action = this->actions.editActions = new QAction(tr("Edit actions..."), this);
      QObject::connect(action, &QAction::triggered, this, &DKCollapsiblePaneTaskMenuExtension::openActionEditor);
   }
}

QAction* DKCollapsiblePaneTaskMenuExtension::preferredEditAction() const {
   return nullptr;
}
QList<QAction*> DKCollapsiblePaneTaskMenuExtension::taskActions() const {
   return QList<QAction*>{
      this->actions.editActions,
   };
}

// action responses:

void DKCollapsiblePaneTaskMenuExtension::openActionEditor() {
   auto* fw = QDesignerFormWindowInterface::findFormWindow(this->widget);
   ActionEditorWindow dialog(this->widget, fw);
   dialog.exec();
}