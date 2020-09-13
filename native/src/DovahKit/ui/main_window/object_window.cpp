#include "object_window.h"
#include "../../editor/open_window_for_form.h"

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
      open_window_for_form(data->stub, this);
   });
}