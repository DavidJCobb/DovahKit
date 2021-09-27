#include "ActionEditorWindow.h"
#include <QAction>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include <QDesignerActionEditorInterface>
#include <QDesignerFormEditorInterface>
#include <QDesignerFormWindowInterface>

ActionEditorWindow::ActionEditorWindow(QWidget* target, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->_target = target;
   //
   {
      auto  actions = target->actions();
      auto* widget  = this->ui.list;
      for (auto* action : actions) {
         auto* item = new QListWidgetItem(action->text(), widget);
         item->setData(ObjectNameRole,    action->objectName());
         item->setData(ActionPointerRole, QVariant::fromValue<QObject*>(action));
         widget->addItem(item);
      }
   }
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, &ActionEditorWindow::commit);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &ActionEditorWindow::cancel);
   //
   QObject::connect(this->ui.buttonNew, &QPushButton::clicked, this, [this]() {
      auto* dialog = new QDialog(this);
      auto* layout = new QGridLayout(dialog);
      dialog->setWindowTitle(tr("Add new action..."));
      //
      layout->addWidget(new QLabel(tr("Object name:"), dialog), 0, 0);
      layout->addWidget(new QLabel(tr("Text:"), dialog), 1, 0);
      //
      auto* name_edit = new QLineEdit(dialog);
      auto* text_edit = new QLineEdit(dialog);
      layout->addWidget(name_edit, 0, 1);
      layout->addWidget(text_edit, 1, 1);
      //
      {
         auto* wrap   = new QWidget(dialog);
         auto* span   = new QBoxLayout(QBoxLayout::Direction::LeftToRight, wrap);
         auto* ok     = new QPushButton(tr("Add"), dialog);
         auto* cancel = new QPushButton(tr("Cancel"), dialog);
         wrap->setLayout(span);
         span->addSpacing(1);
         span->addWidget(ok);
         span->addWidget(cancel);
         layout->addWidget(wrap, 2, 0, 1, 2);
         //
         QObject::connect(ok,     &QPushButton::clicked, dialog, &QDialog::accept);
         QObject::connect(cancel, &QPushButton::clicked, dialog, &QDialog::reject);
      }
      QObject::connect(dialog, &QDialog::accepted, this, [this, name_edit, text_edit]() {
         this->addNewAction(name_edit->text(), text_edit->text());
      });
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      //
      dialog->exec();
   });
   QObject::connect(this->ui.buttonMoveUp,   &QPushButton::clicked, this, &ActionEditorWindow::moveCurrentActionUp);
   QObject::connect(this->ui.buttonMoveDown, &QPushButton::clicked, this, &ActionEditorWindow::moveCurrentActionDown);
   QObject::connect(this->ui.buttonRemove,   &QPushButton::clicked, this, &ActionEditorWindow::removeCurrentAction);
}
void ActionEditorWindow::commit() {
   auto* fw = QDesignerFormWindowInterface::findFormWindow(this->_target);
   assert(fw);
   auto* fe = fw->core();
   assert(fe);
   auto* intfc = fe->actionEditor();
   //
   QVector<QAction*> list;
   {
      auto* widget = this->ui.list;
      int   count  = widget->count();
      list.reserve(count);
      for (int i = 0; i < count; ++i) {
         auto* item = widget->item(i);
         if (item) {
            auto* action = qobject_cast<QAction*>(item->data(ActionPointerRole).value<QObject*>());
            if (!action) {
               action = new QAction;
               action->setText(item->text());
               action->setObjectName(item->data(ObjectNameRole).toString());
               intfc->manageAction(action);
            } else {
               this->_target->removeAction(action);
            }
            list.push_back(action);
         }
      }
   }
   for (auto* action : this->_removals) {
      if (!action)
         continue;
      intfc->unmanageAction(action);
      this->_target->removeAction(action);
      if (!action->parent())
         delete action;
   }
   this->_removals.clear();
   //
   for (auto* action : list) {
      this->_target->addAction(action);
   }
   //
   this->accept();
}
void ActionEditorWindow::cancel() {
   this->reject();
}

void ActionEditorWindow::moveAction(int index, int by) {
   if (by == 0 || index < 0)
      return;
   auto* widget = this->ui.list;
   auto  count  = widget->count();
   if (index - by < 0)
      return;
   if (index + by >= count)
      return;
   auto* item = widget->takeItem(index);
   if (item) {
      int move = index + by;
      widget->insertItem(move, item);
   }
}
void ActionEditorWindow::moveCurrentActionUp() {
   this->moveAction(this->currentIndex(), -1);
}
void ActionEditorWindow::moveCurrentActionDown() {
   this->moveAction(this->currentIndex(), 1);
}
void ActionEditorWindow::removeCurrentAction() {
   int index = this->currentIndex();
   if (index < 0)
      return;
   auto* widget = this->ui.list;
   auto  count  = widget->count();
   if (index >= count)
      return;
   auto* item = widget->takeItem(index);
   if (item) {
      auto* action = qobject_cast<QAction*>(item->data(ActionPointerRole).value<QObject*>());
      if (action) {
         this->_removals.push_back(action);
      }
      delete item;
   }
}
void ActionEditorWindow::addNewAction(QString name, QString text) {
   auto* widget = this->ui.list;
   auto* item   = new QListWidgetItem(text, widget);
   item->setData(ObjectNameRole,    name);
   item->setData(ActionPointerRole, QVariant::fromValue<QObject*>(nullptr));
   int index = this->currentIndex();
   if (index >= 0) {
      widget->insertItem(index + 1, item);
   } else {
      widget->addItem(item);
   }
}