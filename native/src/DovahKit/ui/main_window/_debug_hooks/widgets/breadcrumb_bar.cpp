#include "./breadcrumb_bar.h"
#include <QBoxLayout>
#include <QDialog>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QTreeView>
#include "widgets/DKBreadcrumbBar.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void breadcrumb_bar::execute(QWidget* parent) {
      QDialog* dialog = new QDialog(parent);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      auto* layout = new QVBoxLayout(dialog);
      dialog->setLayout(layout);

      auto* widget = new DKBreadcrumbBar(dialog);
      layout->addWidget(widget);

      auto* model = new QStandardItemModel(dialog);
      widget->setModel(model);
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
            _generate_n_children(root->child(3, 0), 2);
      }

      auto* view = new QTreeView(dialog);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setModel(model);
      layout->addWidget(view);
      {
         auto* sel_model = view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, dialog, [widget](const QItemSelection& selected) {
            if (selected.isEmpty())
               return;
            const QSignalBlocker blocker(widget);
            widget->setCurrentIndex(selected[0].topLeft());
         });
         QObject::connect(widget, &DKBreadcrumbBar::currentIndexChanged, dialog, [sel_model](const QModelIndex& qmi) {
            sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         });
      }

      dialog->show();
   }
}