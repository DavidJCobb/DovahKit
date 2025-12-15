#include "./scoped_proxy_model.h"
#include <array>
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QPushButton>
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

      treeview_source->setAcceptDrops(true);
      treeview_source->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
      treeview_source->setDragDropOverwriteMode(false);
      treeview_source->setDropIndicatorShown(true);

      {  // allow editing of the "with root" tree so we can test dragging the scope to be a sibling of itself
         treeview_proxy_with_root->setAcceptDrops(true);
         treeview_proxy_with_root->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
         treeview_proxy_with_root->setDragDropOverwriteMode(false);
         treeview_proxy_with_root->setDropIndicatorShown(true);
      }

      union {
         std::array<DKScopedProxyModel*, 2> list = {};
         struct {
            DKScopedProxyModel* baseline;
            DKScopedProxyModel* with_root;
         };
      } proxies;
      static_assert(sizeof(proxies.list) == sizeof(proxies));

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
         auto* proxy = proxies.baseline = new DKScopedProxyModel(treeview_proxy);
         treeview_proxy->setModel(proxy);
      }
      {
         auto* proxy = proxies.with_root = new DKScopedProxyModel(treeview_proxy_with_root);
         treeview_proxy_with_root->setModel(proxy);
         proxy->setScopeVisible(true);
      }

      {
         auto* enable = new QCheckBox("Synchronize lower views to top view's selection", dialog);
         layout->addWidget(enable);
         enable->setChecked(true);
         for (auto* proxy : proxies.list) {
            assert(proxy != nullptr);
            proxy->setSourceModel(source);
         }
         auto* sel_model = treeview_source->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, dialog, [proxies, enable](const QItemSelection& sel) {
            if (!enable->isChecked())
               return;
            for (auto* proxy : proxies.list) {
               if (sel.empty())
                  proxy->setScopeIndex(QModelIndex{});
               else
                  proxy->setScopeIndex(sel[0].topLeft());
            }
         });
         QObject::connect(enable, &QCheckBox::toggled, dialog, [proxies, sel_model](bool checked) {
            if (!checked)
               return;
            //
            // when re-checking the box, mass re-sync
            //
            const auto sel = sel_model->selection();
            for (auto* proxy : proxies.list) {
               if (sel.empty())
                  proxy->setScopeIndex(QModelIndex{});
               else
                  proxy->setScopeIndex(sel[0].topLeft());
            }
         });
      }

      {
         auto* button = new QPushButton("Toggle name of selected item", dialog);
         layout->addWidget(button);

         auto* sel_model = treeview_source->selectionModel();
         QObject::connect(button, &QPushButton::clicked, dialog, [source, sel_model]() {
            const auto sel = sel_model->selection();
            if (sel.empty())
               return;
            auto qmi = sel[0].topLeft();

            auto data = source->data(qmi, Qt::ToolTipRole);
            if (data.isValid()) {
               source->setData(qmi, {}, Qt::ToolTipRole);
               source->setData(qmi, data, Qt::DisplayRole);
            } else {
               data = source->data(qmi, Qt::DisplayRole);
               source->setData(qmi, data, Qt::ToolTipRole);
               source->setData(qmi, "Toggled!", Qt::DisplayRole);
            }
         });
      }
      
      dialog->show();
   }
}
