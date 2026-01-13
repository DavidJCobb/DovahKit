#include "./perk.h"
#include "dovah/core.h"
#include "dovah/forms/components/papyrus/fragment_data/perk_fragment_data.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./perk/FormSubdialogPerkEntry.h"
#include "./perk/PerkEntriesModel.h"

FormDialogPerk::FormDialogPerk(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.nextPerk->setAllowedFormType(dovah::form_type::perk);
   ui::set_range<decltype(decltype(loaded_form_type::data)::level)>(this->ui.level);
   ui::set_range<decltype(decltype(loaded_form_type::data)::rank_count)>(this->ui.ranks);

   {
      auto* model  = this->_models.entries = new PerkEntriesModel(this);
      auto* widget = this->ui.entries;
      widget->setModel(model);
      widget->setWordWrap(false);
      ui::typical_tableview_config(widget);
      widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      ui::set_tableview_column_flex(widget, [model](DKHeaderView& header, const QFontMetrics& metrics) {
         std::array<int, PerkEntriesModel::ColumnCount> widths;
         for (size_t i = 0; i < widths.size(); ++i)
            widths[i] = metrics.boundingRect(model->headerData(i, Qt::Orientation::Horizontal, Qt::DisplayRole).toString()).width();

         header.setColumnFlex(PerkEntriesModel::Column::Rank,     0, 0, widths[0]);
         header.setColumnFlex(PerkEntriesModel::Column::Priority, 0, 0, widths[1]);
         header.setColumnFlex(PerkEntriesModel::Column::Type,     0, 0, widths[2]);
         header.setColumnFlex(PerkEntriesModel::Column::Data1,    3, 0, widths[3]);
         header.setColumnFlex(PerkEntriesModel::Column::Data2,    3, 0, widths[4]);
         header.setColumnFlex(PerkEntriesModel::Column::Data3,    1, 0, widths[5]);
         header.setColumnFlex(PerkEntriesModel::Column::Data4,    1, 0, widths[6]);
      });
      
      auto* sel_model = widget->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model](const QItemSelection& sel) {
         if (sel.empty() || sel[0].isEmpty()) {
            this->ui.buttonEntriesEdit->setEnabled(false);
            this->ui.buttonEntriesRemove->setEnabled(false);
            return;
         }
         this->ui.buttonEntriesEdit->setEnabled(true);
         this->ui.buttonEntriesRemove->setEnabled(true);
      });
      QObject::connect(this->ui.buttonEntriesAdd, &QPushButton::clicked, this, [this, model, sel_model]() {
         FormSubdialogPerkEntry modal(*this->form, this);
         if (modal.exec() == QDialog::Accepted) {
            model->addItem(modal.value());
         }
      });
      QObject::connect(this->ui.buttonEntriesEdit, &QPushButton::clicked, this, [this, model, sel_model]() {
         auto sel = sel_model->selectedRows();
         if (sel.empty())
            return;
         auto row   = sel[0].row();
         auto value = model->item(row);
         FormSubdialogPerkEntry modal(*this->form, this);
         modal.setValue(value);
         if (modal.exec() == QDialog::Accepted) {
            value = modal.value();
            model->setItem(row, value);
         }
      });
      QObject::connect(this->ui.buttonEntriesRemove, &QPushButton::clicked, this, [this, model, sel_model]() {
         auto sel = sel_model->selectedRows();
         if (sel.empty())
            return;
         auto row = sel[0].row();
         model->deleteItem(row);
      });
   }

   this->load(); // this creates the working copy.
}
void FormDialogPerk::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.flagTrait,    working.data.is_trait);
   ui::bind(this->ui.flagPlayable, working.data.playable);
   ui::bind(this->ui.flagHidden,   working.data.hidden);
   ui::bind(this->ui.level, working.data.level);
   ui::bind(this->ui.ranks, working.data.rank_count);
   this->ui.icon->setValue(ui::types::game_file_path("Data\\Textures\\").append(QString::fromStdString(working.icon)));
   ui::bind(this->ui.nextPerk, working.next_perk, working);
   this->ui.description->setPlainText(gls.convert_localized_string(working.description));

   this->ui.conditions->importFrom(working, working.conditions);

   {
      auto entries = ui::types::perk_entries::entry::pull_list_from_backend(working);
      this->_models.entries->initializeFrom(entries);
      this->ui.buttonEntriesEdit->setEnabled(!entries.empty());
      this->ui.buttonEntriesRemove->setEnabled(!entries.empty());
   }

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogPerk::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   // Easiest way to add the perk entries in is to just clear the whole form 
   // and then write everything back in.
   working.clear();
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   {
      auto path = this->ui.icon->value();
      auto str  = path.lexically_relative("Data\\Textures\\").to_string().toStdString();
      working.icon = str;
   }
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());

   this->ui.conditions->exportTo(working, working.conditions);

   {
      const size_t count = this->_models.entries->rowCount();
      for (size_t i = 0; i < count; ++i) {
         auto item = this->_models.entries->item(i);
         item.append_into_backend(working);
      }
   }

   this->ui.scriptListPane->commit();
}