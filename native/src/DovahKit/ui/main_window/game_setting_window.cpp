#include "game_setting_window.h"
#include <QItemSelectionModel>

namespace {
   GameSettingList::model_item_type* _get_selected_item(QTableView* widget) {
      auto* proxy  = (QSortFilterProxyModel*)widget->model();
      auto  select = widget->selectionModel()->selection().indexes();
      if (!select.size())
         return nullptr;
      auto  real   = proxy->mapToSource(select[0]); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return nullptr;
      return (GameSettingList::model_item_type*)real.internalPointer();
   }
}
GameSettingWindow::GameSettingWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->ui.list->setTextFilter(this->ui.filter);
   this->ui.list->build();
   QObject::connect(this->ui.list->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.settingName->setText(tr("No setting selected", "game setting window"));
      this->ui.description->setText("");
      //
      this->ui.valueString->setPlainText("");
      //
      this->ui.valueFloat->setDisabled(true);
      this->ui.valueInt->setDisabled(true);
      this->ui.valueString->setDisabled(true);
      this->ui.buttonReset->setDisabled(true);
      //
      const auto* item = _get_selected_item(this->ui.list);
      if (item) {
         QWidget* value = nullptr;
         switch (item->type) {
            case dovah::game_setting_type::float32:
               value = this->ui.valueFloat;
               this->ui.valueFloat->setValue(item->value.float32);
               break;
            case dovah::game_setting_type::integer:
               value = this->ui.valueInt;
               this->ui.valueInt->setValue(item->value.number);
               break;
            case dovah::game_setting_type::string:
               value = this->ui.valueString;
               this->ui.valueString->setPlainText(item->value.string);
               break;
         }
         if (value) {
            value->setEnabled(true);
            this->ui.buttonReset->setEnabled(true);
         }
         //
         this->ui.settingName->setText(item->name);
         this->ui.description->setText(item->description);
      }
   });
}