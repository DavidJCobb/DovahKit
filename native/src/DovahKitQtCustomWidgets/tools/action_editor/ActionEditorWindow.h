#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_ActionEditorWindow.h"

class QDesignerFormEditorInterface;

class ActionEditorWindow : public QDialog {
   Q_OBJECT;
   public:
      ActionEditorWindow(QWidget* target, QWidget* parent = Q_NULLPTR);

      static constexpr auto ActionPointerRole = Qt::ItemDataRole(Qt::UserRole);     // for already-existing actions
      static constexpr auto ObjectNameRole    = Qt::ItemDataRole(Qt::UserRole + 1); // for yet-to-be-created actions

      inline QWidget* target() const noexcept { return this->_target; }

      inline int currentIndex() const noexcept { return this->ui.list->currentRow(); }
      
   private:
      Ui::ActionEditorWindow ui;
      QWidget* _target = nullptr;

      QList<QAction*> _removals;

   public slots:
      void commit();
      void cancel();
      void moveAction(int index, int by);
      void moveCurrentActionUp();
      void moveCurrentActionDown();
      void removeCurrentAction();
      void addNewAction(QString name, QString text);
};