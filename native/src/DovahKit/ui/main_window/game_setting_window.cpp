#include "game_setting_window.h"
#include <QItemSelectionModel>
#include "../../dovah/data/game_settings.h"
#include "../../helpers/qt/spinbox.h"

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
   cobb::qt::remove_spinbox_bounds(this->ui.valueF);
   cobb::qt::remove_spinbox_bounds(this->ui.valueI);
   //
   this->ui.list->setTextFilter(this->ui.filter);
   this->ui.list->build();
   QObject::connect(this->ui.list->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.settingName->setText(tr("No setting selected", "game setting window"));
      this->ui.description->setText("");
      this->ui.valueStack->setCurrentWidget(this->ui.vpNone);
      //
      this->ui.buttonReset->setEnabled(false);
      //
      const auto* item = _get_selected_item(this->ui.list);
      if (item) {
         QWidget* page = nullptr;
         switch (item->type) {
            case dovah::game_setting_type::boolean:
               page = this->ui.vpBool;
               this->ui.valueB->setCurrentIndex(item->value.boolean ? 1 : 0);
               break;
            case dovah::game_setting_type::float32:
               page = this->ui.vpFloat;
               this->ui.valueF->setValue(item->value.float32);
               break;
            case dovah::game_setting_type::integer:
               page = this->ui.vpInt;
               this->ui.valueI->setValue(item->value.number);
               break;
            case dovah::game_setting_type::string:
               page = this->ui.vpString;
               this->ui.valueS->setPlainText(item->value.string);
               break;
         }
         if (page) {
            this->ui.valueStack->setCurrentWidget(page);
            this->ui.buttonReset->setEnabled(true);
         } else {
            this->ui.valueStack->setCurrentWidget(this->ui.vpUnknown);
         }
         //
         this->ui.settingName->setText(item->name);
         this->ui.description->setText(item->description);
      }
   });
}