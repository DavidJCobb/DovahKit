#include "./message.h"
#include "dovah/core.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"
#include "ui/utils/typical_tableview_config.h"
#include "./message/MessageButtonsModel.h"

FormDialogMessage::FormDialogMessage(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.owningQuest->setAllowedFormType(dovah::form_type::quest);
   QObject::connect(this->ui.owningQuest, &DKFormPicker::formChanged, this->ui.buttonConditions, &DKConditionList::overrideOwningForm);

   {
      auto* model = this->_models.buttons = new MessageButtonsModel(this);
      auto* view  = this->ui.buttonsView;
      view->setModel(model);
      ui::typical_tableview_config(view);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setWordWrap(false);

      auto* sel_model = view->selectionModel();
      QObject::connect(
         sel_model,
         &QItemSelectionModel::selectionChanged,
         this,
         [this, sel_model, model](const QItemSelection& selected, const QItemSelection& deselected) {
            auto*      widget  = this->ui.currentButtonText;
            const auto blocker = QSignalBlocker(widget);

            auto rows = sel_model->selectedRows();
            if (rows.empty()) {
               widget->setEnabled(false);
               widget->setText({});
               this->ui.buttonConditions->setEnabled(false);
               this->ui.buttonConditions->clear();
               return;
            }
            auto* item = model->item(rows[0].row());

            widget->setEnabled(true);
            widget->setText(item->text);

            this->ui.buttonConditions->setEnabled(true);
            if (!deselected.isEmpty()) {
               auto qmi = deselected[0].topLeft();
               this->_push_button_conditions(qmi.row());
            }
            this->ui.buttonConditions->importFrom(*this->form, item->conditions);
         }
      );
      this->ui.currentButtonText->setEnabled(false);
      this->ui.buttonConditions->setEnabled(false);
   }

   this->load(); // this creates the working copy.
}
void FormDialogMessage::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.title->setText(editor.convert_localized_string(working.name));
   ui::bind(this->ui.owningQuest, working.owning_quest, working);
   ui::bind(this->ui.displayTime, working.display_time);
   ui::bind(this->ui.flagMessageBox,  working.flags, loaded_form_type::flag::message_box);
   ui::bind(this->ui.flagAutoDisplay, working.flags, loaded_form_type::flag::auto_display);
   this->ui.text->setPlainText(editor.convert_localized_string(working.description));

   {
      auto* model    = this->_models.buttons;
      auto& src_list = working.buttons;
      std::vector<MessageButtonsModel::node_type> dst_list;
      dst_list.reserve(src_list.size());
      for (auto& src : src_list) {
         auto& dst = dst_list.emplace_back();
         dst.text = editor.convert_localized_string(src.text);
         dst.conditions.reserve(src.conditions.size());
         for (auto& cnd : src.conditions)
            dst.conditions.emplace_back(cnd);
      }
      model->overwriteAllItems(dst_list);
   }
}
void FormDialogMessage::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.title->text());
   editor.assign_localized_string(working.description, this->ui.text->toPlainText());

   this->_push_button_conditions(this->_selected_button_row());

   {
      auto* model = this->_models.buttons;
      int   count = model->rowCount();
      {
         auto& list = working.buttons;
         for (auto& item : list)
            item.conditions.clear(working);
         list.clear();
         list.reserve(count);
      }
      for (int i = 0; i < count; ++i) {
         auto* src = model->item(i);
         if (!src)
            continue;
         auto& dst = working.buttons.emplace_back();
         editor.assign_localized_string(dst.text, src->text);
         dst.conditions.append_all_of(working, src->conditions);
      }
   }
}

int FormDialogMessage::_selected_button_row() const {
   auto* sel_model = this->ui.buttonsView->selectionModel();
   auto  rows      = sel_model->selectedRows();
   if (rows.isEmpty())
      return -1;
   return rows[0].row();
}
void FormDialogMessage::_push_button_conditions(int row) {
   auto* model = this->_models.buttons;
   auto* prior = model->item(row);
   if (prior) {
      auto to_update = *prior;
      this->ui.buttonConditions->exportTo(*this->form, to_update.conditions);
      model->overwrite(row, to_update);
   }
}