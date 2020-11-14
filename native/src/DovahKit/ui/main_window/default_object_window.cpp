#include "default_object_window.h"
#include <QAction>
#include <QItemSelectionModel>
#include <QMenu>
#include "../../dovah/data/default_objects.h"
#include "../../editor/core.h"
#include "../../editor/get_default_object_info.h"

namespace {
   DefaultObjectList::model_item_type* _get_selected_item(QTableView* widget) {
      auto* proxy  = (QSortFilterProxyModel*)widget->model();
      auto  select = widget->selectionModel()->selection().indexes();
      if (!select.size())
         return nullptr;
      auto  real   = proxy->mapToSource(select[0]); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return nullptr;
      return (DefaultObjectList::model_item_type*)real.internalPointer();
   }
}
DefaultObjectWindow::DefaultObjectWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   {
      auto* widget  = this->ui.entryName;
      auto  metrics = QFontMetrics(widget->font());
      int   longest = 0;
      int   length  = 0;
      for_each_default_object([this, &length, &longest, &metrics](uint32_t signature, const QString& name, const QString& desc) {
         int size = name.size();
         if (size < length - 5) // silly shortcut to avoid having to compute font metrics for *every* name
            return false;
         int width = metrics.boundingRect(name).width();
         if (width > longest) {
            longest = width;
            length  = size;
         }
         return false;
      });
      widget->setMinimumWidth(longest);
   }
   //
   QObject::connect(this->ui.buttonCommit, &QPushButton::clicked, this, [this]() {
      const auto* item = _get_selected_item(this->ui.list);
      if (!item)
         return;
      auto& editor = DovahKitCore::get();
      editor.set_default_object(item->signature, this->ui.form->formStub());
   });
   //
   this->ui.list->setTextFilter(this->ui.filter);
   this->ui.list->build();
   this->ui.form->setAllowNone(true);
   this->ui.buttonCommit->setEnabled(false);
   QObject::connect(this->ui.list->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.entryName->setText(tr("No entry selected", "default object window"));
      this->ui.description->setText("");
      this->ui.form->setEnabled(false);
      //
      this->ui.buttonCommit->setEnabled(false);
      //
      const auto* item = _get_selected_item(this->ui.list);
      if (item) {
         dovah::form_type_t allowed_type = item->form_type;
         if (allowed_type) {
            if (!this->ui.form->allowsFormType(allowed_type)) {
               this->ui.form->setAllowedFormType(allowed_type);
               this->ui.form->populate();
            }
         } else {
            this->ui.form->populate();
         }
         if (item->form)
            this->ui.form->setFormByID(item->form->formID);
         else
            this->ui.form->setFormByID(0);
         //
         bool has_data = DovahKitCore::get().has_data();
         this->ui.buttonCommit->setEnabled(has_data);
         this->ui.form->setEnabled(has_data);
         //
         this->ui.entryName->setText(item->name);
         this->ui.description->setText(item->description);
      }
   });
}