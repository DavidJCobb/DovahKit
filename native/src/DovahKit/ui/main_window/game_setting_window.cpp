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
   {
      auto* widget  = this->ui.settingName;
      auto  metrics = QFontMetrics(widget->font());
      int   longest = 0;
      int   length  = 0;
      for (auto& definition : dovah::game_settings) {
         int size  = strlen(definition.name);
         if (size < length - 5) // silly shortcut to avoid having to compute font metrics for *every* setting name
            continue;
         int width = metrics.boundingRect(definition.name).width();
         if (width > longest) {
            longest = width;
            length  = size;
         }
      }
      widget->setMinimumWidth(longest);
   }
   //
   QObject::connect(this->ui.buttonCommit, &QPushButton::clicked, this, [this]() {
      const auto* item = _get_selected_item(this->ui.list);
      if (!item)
         return;
      auto& editor = DovahKitCore::get();
      dovah::game_setting_value value;
      std::string name = item->name.toStdString();
      switch (item->type) {
         case dovah::game_setting_type::boolean:
            value.b = this->ui.valueB->currentIndex() != 0;
            break;
         case dovah::game_setting_type::float32:
            value.f = this->ui.valueF->value();
            break;
         case dovah::game_setting_type::integer:
            value.i = this->ui.valueF->value();
            break;
         case dovah::game_setting_type::string:
            editor.assign_localized_string(value.s, this->ui.valueS->plainText());
            break;
         default:
            return;
      }
      bool result = editor.edit_game_setting(name.c_str(), value);
      if (!result) {
         //
         // TODO: message box telling the user that an error occurred, and to check the log window
         //
      }
   });
   QObject::connect(this->ui.buttonReset, &QPushButton::clicked, this, [this]() {
      //
      // TODO
      //
   });
   //
   this->ui.list->setTextFilter(this->ui.filter);
   this->ui.list->build();
   QObject::connect(this->ui.list->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.settingName->setText(tr("No setting selected", "game setting window"));
      this->ui.description->setText("");
      this->ui.valueStack->setCurrentWidget(this->ui.vpNone);
      //
      this->ui.buttonCommit->setEnabled(false);
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
            this->ui.buttonCommit->setEnabled(true);
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