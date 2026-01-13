#include "./game_setting_window.h"
#include <QAction>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include "helpers/qt/spinbox.h"
#include "dovah/data/game_settings.h"
#include "dovah/exceptions/game_setting_value_change_failed.h"
#include "editor/core.h"
#include "editor/subsystems/game_localized_strings/core.h"

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
   #pragma region Context menu
   this->_formActionRenumber = new QAction(tr("Change form ID...", "game setting window"), this);
   QObject::connect(this->_formActionRenumber, &QAction::triggered, this, [this]() {
      const auto* item = _get_selected_item(this->ui.list);
      if (item)
         DovahKitCore::get().renumber_game_setting(item->name.toStdString().c_str(), this);
   });
   //
   this->ui.list->setContextMenuPolicy(Qt::CustomContextMenu);
   QObject::connect(this->ui.list, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
      auto opener = this->ui.list;
      //
      auto* item = _get_selected_item(opener);
      this->_formActionRenumber->setVisible(item != nullptr);
      this->_formActionRenumber->setEnabled(item && item->is_in_active_file);
      //
      QMenu menu(opener);
      menu.addAction(this->_formActionRenumber);
      if (menu.isEmpty())
         return;
      menu.exec(opener->mapToGlobal(pos));
   });
   #pragma endregion
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
            {
               auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
               gls.assign_localized_string(value.s, this->ui.valueS->toPlainText());
            }
            break;
         default:
            return;
      }
      {
         using exception  = dovah::exceptions::game_setting_value_change_failed;
         using error_code = exception::error_code;

         try {
            editor.edit_game_setting(name.c_str(), value);
         } catch (const exception& ex) {
            QString text;
            switch (ex.code) {
               case error_code::form_id_is_occupied:
                  text = tr("The requested form ID is occupied.");
                  break;
               case error_code::form_id_is_reserved:
                  text = tr("The requested form ID is reserved for use by an in-progress editing operation.");
                  break;
               case error_code::form_id_is_zero:
                  text = tr("Zero is not a valid form ID to use here.");
                  break;
               case error_code::no_active_file:
                  text = tr("There is no active file, nor room in the load order for a new file.");
                  break;
               case error_code::no_form_id_available:
                  text = tr("There are no available form IDs to use for this game setting right now.");
                  break;
               default:
                  text = tr("An internal error occurred. The game setting's value was not changed.");
                  break;
            }
            QMessageBox::critical(this, tr("Error"), text);
         }
      }
   });
   //
   this->ui.list->setTextFilter(this->ui.filter);
   this->ui.list->build();
   this->ui.valueStack->setCurrentWidget(this->ui.vpNone);
   this->ui.buttonCommit->setEnabled(false);
   QObject::connect(this->ui.list->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.settingName->setText(tr("No setting selected", "game setting window"));
      this->ui.description->setText("");
      this->ui.valueStack->setCurrentWidget(this->ui.vpNone);
      //
      this->ui.buttonCommit->setEnabled(false);
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
            //
            bool has_data = DovahKitCore::get().has_data();
            //
            this->ui.buttonCommit->setEnabled(has_data);
         } else {
            this->ui.valueStack->setCurrentWidget(this->ui.vpUnknown);
         }
         //
         this->ui.settingName->setText(item->name);
         this->ui.description->setText(item->description);
      }
   });
}