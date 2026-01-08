#include "./breadcrumb_bar.h"
#include <array>
#include <QBoxLayout>
#include <QDialog>
#include <QIcon>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include "widgets/DKBreadcrumbBar.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void breadcrumb_bar::execute(QWidget* parent) {
      QDialog* dialog = new QDialog(parent);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      auto* layout = new QVBoxLayout(dialog);
      dialog->setLayout(layout);

      auto* widget       = new DKBreadcrumbBar(dialog);
      auto* disabled_bar = new DKBreadcrumbBar(dialog);
      auto* bar_rtl      = new DKBreadcrumbBar(dialog);
      const auto bars = std::array{
         widget,
         disabled_bar,
         bar_rtl,
      };
      for (auto* bar : bars)
         layout->addWidget(bar);

      bar_rtl->setLayoutDirection(Qt::LayoutDirection::RightToLeft);

      auto* model = new QStandardItemModel(dialog);
      widget->setModel(model);
      disabled_bar->setModel(model);
      bar_rtl->setModel(model);
      {
         int unique_id = 0;

         QStandardItem* root   = model->invisibleRootItem();
         QStandardItem* parent = root;

         auto _generate_n_children = [&unique_id](QStandardItem* parent, int count) {
            for (int i = 0; i < count; ++i) {
               char c = 'A' + unique_id++;
               QStandardItem* item = new QStandardItem(QString(c));
               parent->appendRow(item);
            }
         };

         _generate_n_children(root, 4);
            _generate_n_children(root->child(1, 0), 3);
            _generate_n_children(root->child(2, 0), 5);
               _generate_n_children(root->child(2, 0)->child(3, 0), 1);
                  _generate_n_children(root->child(2, 0)->child(3, 0)->child(0, 0), 4);
                  {
                     QPixmap pixmap(16, 16);
                     pixmap.fill(QColor(255, 0, 0));
                     QIcon icon(pixmap);
                     root->child(2, 0)->child(3, 0)->child(0, 0)->child(2, 0)->setIcon(icon);
                  }
            _generate_n_children(root->child(3, 0), 2);

         //
         // Test eliding when names get long.
         //
         {
            QStandardItem* a = new QStandardItem("This is a really long name");
            root->appendRow(a);

            QStandardItem* b = new QStandardItem("Big big big name");
            a->appendRow(b);

            QStandardItem* c = new QStandardItem("Shorter");
            b->appendRow(c);

            QStandardItem* d = new QStandardItem("Lorem ipsum lorem ipsum something something");
            c->appendRow(d);

            QStandardItem* e = new QStandardItem("Short");
            d->appendRow(e);
         }
      }

      auto* view = new QTreeView(dialog);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setModel(model);
      layout->addWidget(view);
      {
         auto* sel_model = view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, dialog, [bars](const QItemSelection& selected) {
            if (selected.isEmpty())
               return;
            auto qmi = selected[0].topLeft();
            for (auto* bar : bars) {
               const QSignalBlocker blocker(bar);
               bar->setCurrentIndex(qmi);
            }
         });
         for (auto* bar : bars) {
            QObject::connect(bar, &DKBreadcrumbBar::currentIndexChanged, dialog, [sel_model](const QModelIndex& qmi) {
               sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            });
         }
      }

      {
         auto* button = new QPushButton("Disable in 5s, for 10s", dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, button, [button, disabled_bar]() {
            button->setEnabled(false);
            button->setText("Will disable...");
            QTimer::singleShot(5 * 1000, button, [button, disabled_bar]() {
               disabled_bar->setEnabled(false);
               button->setText("Disabled...");
               QTimer::singleShot(10 * 1000, button, [button, disabled_bar]() {
                  button->setEnabled(true);
                  button->setText("Disable in 5s, for 10s");
                  disabled_bar->setEnabled(true);
               });
            });
         });
      }

      {
         auto* button = new QPushButton("Toggle whether the buttons have a menu", dialog);
         auto* menu   = new QMenu(dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, button, [bars, menu]() {
            if (bars[0]->rootMenu()) {
               for (auto* bar : bars)
                  bar->setRootMenu(nullptr);
            } else {
               for (auto* bar : bars)
                  bar->setRootMenu(menu);
            }
         });
         menu->addAction("Dummy action 1");
         menu->addAction("Dummy action 2");
      }

      dialog->show();
   }
}