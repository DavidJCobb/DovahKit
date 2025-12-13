#include "./scoped_proxy_model.h"
#include <QDialog>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QTreeView>
#include "ui/models/DKScopedProxyModel.h"

namespace DovahKitDebug::features::models {
   /*static*/ void scoped_proxy_model::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      auto* treeview_source = new QTreeView(dialog);
      auto* treeview_proxy = new QTreeView(dialog);
      auto* treeview_proxy_with_root = new QTreeView(dialog);
      layout->addWidget(treeview_source);
      layout->addWidget(treeview_proxy);
      layout->addWidget(treeview_proxy_with_root);

      QStandardItemModel* source = new QStandardItemModel(treeview_source);
      treeview_source->setModel(source);
      {
         int unique_id = 0;

         QStandardItem* root   = source->invisibleRootItem();
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
      }
      {
         auto* proxy = new DKScopedProxyModel(treeview_proxy);
         treeview_proxy->setModel(proxy);
         proxy->setSourceModel(source);

         auto* sel_model = treeview_source->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, proxy, [proxy](const QItemSelection& sel) {
            if (sel.empty()) {
               proxy->setRootIndex(QModelIndex{});
               return;
            }
            proxy->setRootIndex(sel[0].topLeft());
         });
      }
      {
         auto* proxy = new DKScopedProxyModel(treeview_proxy_with_root);
         treeview_proxy_with_root->setModel(proxy);
         proxy->setRootVisible(true);
         proxy->setSourceModel(source);

         auto* sel_model = treeview_source->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, proxy, [proxy](const QItemSelection& sel) {
            if (sel.empty()) {
               proxy->setRootIndex(QModelIndex{});
               return;
            }
            proxy->setRootIndex(sel[0].topLeft());
         });
      }
      
      dialog->show();
   }
}
