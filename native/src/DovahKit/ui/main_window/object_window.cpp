#include "object_window.h"
#include <QMenu>
#include "../../editor/open_window_for_form.h"
#include "form_use_info.h"

ObjectWindow::ObjectWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   this->ui.table->setSource(this->ui.tree);
   this->ui.table->setFilter(this->ui.filter);
   QObject::connect(this->ui.table, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* proxy = (QSortFilterProxyModel*)this->ui.table->model();
      auto  real  = proxy->mapToSource(index); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return;
      using item_type = FormTable::model_item_type;
      //
      auto data = (item_type*)real.internalPointer();
      if (!data || !data->stub)
         return;
      open_edit_dialog_for_form(data->stub, this);
   });
   //
   this->_formActionShowUseInfo = new QAction(tr("Use Info...", "object window form actions"), this->ui.table);
   QObject::connect(this->_formActionShowUseInfo, &QAction::triggered, [this]() {
      auto  widget = this->ui.table;
      auto* proxy  = (QSortFilterProxyModel*)widget->model();
      auto  select = widget->selectionModel()->selection().indexes();
      if (!select.size())
         return;
      auto  real   = proxy->mapToSource(select[0]); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return;
      using item_type = FormTable::model_item_type;
      //
      auto data = (item_type*)real.internalPointer();
      if (!data || !data->stub)
         return;
      //
      open_use_info_dialog_for_form(data->stub, this->parentWidget());
   });
   this->ui.table->setContextMenuPolicy(Qt::CustomContextMenu);
   QObject::connect(this->ui.table, &QWidget::customContextMenuRequested, [this](const QPoint& pos) {
      auto opener = this->ui.table;
      //
      QMenu menu(opener);
      menu.addAction(this->_formActionShowUseInfo);
      menu.exec(opener->mapToGlobal(pos));
   });
}