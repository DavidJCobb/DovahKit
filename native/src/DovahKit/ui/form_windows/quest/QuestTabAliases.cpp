#include "./QuestTabAliases.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "./QuestAliasesModel.h"
#include "./FormSubdialogQuestLocAlias.h"
#include "./FormSubdialogQuestRefAlias.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/show_parent_scoped_modal.h"
#include "ui/utils/shrink_dialog_on_show.h"
#include "ui/utils/size_tableview_columns.h"
#include "ui/utils/typical_tableview_config.h"

QuestTabAliases::QuestTabAliases(quest_form_type& quest, QWidget* parent) : QObject(parent), working_quest(quest) {
   this->_model = new QuestAliasesModel(quest, this);
}
QuestTabAliases::~QuestTabAliases() {
}
void QuestTabAliases::setupUi() {
   this->ui.view->setModel(this->_model);
   this->ui.view->setWordWrap(false);
   ui::set_custom_context_menu(*this->ui.view, this->ui.context.menu);
   ui::typical_tableview_config(this->ui.view);
   ui::size_tableview_columns<std::array<ui::tableview_column_spec, QuestAliasesModel::ColumnCount>{
      ui::tableview_column_spec{ // Name
         .grow = 2,
      },
      ui::tableview_column_spec{ // ID
         .grow   = 0,
         .shrink = 0,
         .width  = 4,
      },
      ui::tableview_column_spec{ // Optional
         .grow   = 0,
         .shrink = 0,
      },
      ui::tableview_column_spec{ // Type
         .grow   = 0,
         .shrink = 0,
      },
      {}, // Fill
      ui::tableview_column_spec{ // Flags
         .grow   = 0,
         .shrink = 0,
      },
   }>(this->ui.view);

   {  // Context menu
      auto& menu    = this->ui.context.menu;
      auto& actions = this->ui.context.actions;
      {
         auto* action = actions.create_loc = new QAction(tr("New Location Alias"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto qmi = this->_model->createLocAlias();
            if (!qmi.isValid())
               return;
            {
               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(QuestAliasesModel::ColumnCount - 1);
               this->ui.view->selectionModel()->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
            this->edit_selected_alias();
         });
      }
      {
         auto* action = actions.create_loc = new QAction(tr("New Reference Alias"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto qmi = this->_model->createRefAlias();
            if (!qmi.isValid())
               return;
            {
               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(QuestAliasesModel::ColumnCount - 1);
               this->ui.view->selectionModel()->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
            this->edit_selected_alias();
         });
      }
      {
         auto* action = actions.remove = new QAction(tr("Delete"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto rows = this->ui.view->selectionModel()->selectedRows();
            if (rows.empty())
               return;
            this->_model->deleteAlias(rows[0]);
         });
      }
   }

   QObject::connect(this->ui.view, &QAbstractItemView::doubleClicked, this, [this](const QModelIndex& qmi) {
      this->edit_selected_alias();
   });
}

void QuestTabAliases::edit_selected_alias() {
   auto rows = this->ui.view->selectionModel()->selectedRows();
   if (rows.empty())
      return;
   auto  qmi   = rows[0];
   auto* alias = this->_model->alias(qmi);
   if (!alias)
      return;

   //
   // Use a parent-scoped modal rather than application-scoped or window-hierarchy-scoped. 
   // There are windows we don't want to block:
   // 
   //  - The Render Window, for picking refs to fill a ref-alias with.
   // 
   //  - The Object Window, for dragging forms in.
   //
   auto _show_dialog = [this]<typename Dialog>(auto&&... args) {
      auto* parent = this->ui.view->window();

      auto* dialog = new Dialog(args...);
      ui::shrink_dialog_on_show(*dialog);
      dialog->load();
      QObject::connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
         dialog->deleteLater();
         if (result == QDialog::DialogCode::Accepted)
            dialog->save();
      });
      ui::show_parent_scoped_modal(*dialog, parent);
   };

   using alias_type = dovah::loaded_forms::Alias::alias_type;
   if (alias->type == alias_type::reference) {
      _show_dialog.operator()<FormSubdialogQuestRefAlias>(this->working_quest, *(dovah::loaded_forms::ReferenceAlias*)alias);
   } else if (alias->type == alias_type::location) {
      _show_dialog.operator()<FormSubdialogQuestLocAlias>(this->working_quest, *(dovah::loaded_forms::LocationAlias*)alias);
   } else {
      return;
   }
   this->_model->onAliasChanged(qmi);
}