#include "ui_collapsible_pane.h"
#include <QAction>
#include <QBoxLayout>
#include <QDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include "../../../widgets/DKCollapsiblePane.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_collapsible_pane::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      dialog->setLayout(layout);
      //
      auto* pane = new DKCollapsiblePane(dialog);
      pane->setTitle("Test panel");
      layout->addWidget(pane);
      //
      if (auto* view = pane->viewport()) {
         auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, view);
         view->setLayout(layout);
         layout->addWidget(new QPushButton("Filler 1"));
         layout->addWidget(new QPushButton("Filler 2"));
         layout->addWidget(new QPushButton("Filler 3"));
         layout->addWidget(new QPushButton("Filler 4"));
      }
      //
      {
         auto* button = new QPushButton("Add Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [dialog, pane]() {
            auto  count  = pane->actions().count();
            auto* action = new QAction(QString("Action #%1").arg(count));
            action->setProperty("number", count);
            pane->addAction(action);
            //
            QObject::connect(action, &QAction::triggered, [dialog, count]() {
               QMessageBox::information(dialog, QString("Test"), QString("Triggered action %1.").arg(count));
            });
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Remove First Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            auto* target = list.first();
            if (target) {
               pane->removeAction(target);
               delete target;
            }
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Remove Middle Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            int   i      = list.size() / 2;
            auto* target = list[i];
            if (target) {
               pane->removeAction(target);
               delete target;
            }
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Remove Last Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            auto* target = list.back();
            if (target) {
               pane->removeAction(target);
               delete target;
            }
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Scramble Actions");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            auto size = list.size();
            std::srand(std::time(0));
            int offset = std::rand() % size;
            int shift  = std::rand() % (size / 2);
            for (int i = 0; i < size; ++i) {
               int from = (offset + i) % size;
               int to   = (from + shift) % size;
               pane->insertAction(list[to], list[from]);
            }
         });
         layout->addWidget(button);
      }
      //
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->show();
   }
}
