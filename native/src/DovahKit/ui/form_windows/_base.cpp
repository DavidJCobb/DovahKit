#include "_base.h"
#include "../../editor/core.h"
#include "../../dovah/forms/Form.h"
#include "../../dovah/forms/components/extra_data/_templates.h"

#pragma region FormDialogBaseTemplate
FormDialogBaseTemplate::FormDialogBaseTemplate(dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {}
void FormDialogBaseTemplate::load() {
   if (!this->stub)
      return;
   this->_load_impl();
}
void FormDialogBaseTemplate::save() {
   if (!this->stub)
      return;
   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(this->stub);
   this->stub->set_edited(true);
   this->_save_impl();
   emit editor.formModified(this->stub);
}

void FormDialogBaseTemplate::save_form_id(dovah::form_reference_t& target, dovah::bare_form_id_t id) {
   auto& editor = DovahKitCore::get();
   auto* value  = editor.get_form(id);
   this->save_form_id(target, value);
}
void FormDialogBaseTemplate::save_form_id(dovah::form_reference_t& target, dovah::form_stub* value) {
   assert(this->stub->form && "");
   target.set(*this->stub->form, value);
}

void FormDialogBaseTemplate::save_extra_form(dovah::bare_form_id_t formID, extra_data_list& extra, extra_data_type et, bool remove_if_no_form) {
   using dummy_t = dovah::loaded_forms::components::formID_extra_data<0, extra_data_type::action>;
   //
   if (formID || !remove_if_no_form) {
      auto* data = extra.get_or_create_by_type(et);
      if (data)
         this->save_form_id(((dummy_t*)data)->form, formID);
   } else {
      extra.remove_by_type(et);
   }
}
void FormDialogBaseTemplate::save_extra_form(dovah::form_stub* stub, extra_data_list& extra, extra_data_type et, bool remove_if_no_form) {
   using dummy_t = dovah::loaded_forms::components::formID_extra_data<0, extra_data_type::action>;
   //
   if (stub || !remove_if_no_form) {
      auto* data = extra.get_or_create_by_type(et);
      if (data)
         this->save_form_id(((dummy_t*)data)->form, stub);
   } else {
      extra.remove_by_type(et);
   }
}
#pragma endregion

#pragma region FormDialogWorkingCopyBase
FormDialogWorkingCopyBase::FormDialogWorkingCopyBase(dovah::form_type_t ft, dovah::form_stub* stub, QWidget* parent) : QDialog(parent), _allowed_form_type(ft) {
   if (stub->formType == ft) {
      this->stub = stub;
      this->form = this->stub->load();
   }
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->form  = nullptr;
      this->stub  = nullptr;
      this->clone = nullptr;
      QDialog::reject();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub == this->stub) {
         this->form  = nullptr;
         this->stub  = nullptr;
         this->clone = nullptr;
         QDialog::reject();
         return;
      }
      if (this->clone) {
         this->clone->sever_outbound_references_to(*stub); // TODO: have the file load order do this
      }
   });
   //
   QObject::connect(&editor, &DovahKitCore::dataSaveImminent, this, [this]() {
      this->form = nullptr;
   });
   auto _reload = [this]() { this->form = this->stub->load(); };
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete, this, _reload);
   QObject::connect(&editor, &DovahKitCore::dataSaveFailed,   this, _reload);
}
FormDialogWorkingCopyBase::~FormDialogWorkingCopyBase() {
   this->form  = nullptr;
   this->clone = nullptr;
   if (this->stub) {
      this->stub->delete_working_copy();
      this->stub = nullptr;
   }
}

void FormDialogWorkingCopyBase::accept() {
   this->save();
   QDialog::accept();
}
void FormDialogWorkingCopyBase::reject() {
   auto& editor = DovahKitCore::get();
   //
   emit editor.formWorkingCopyDeleteImminent(this->stub);
   this->clone = nullptr;
   this->stub->delete_working_copy();
   emit editor.formWorkingCopyDeleteComplete(this->stub);
   this->form = nullptr;
   this->stub = nullptr;
   //
   QDialog::reject();
}

void FormDialogWorkingCopyBase::load() {
   if (!this->stub)
      return;
   this->form  = this->stub->load();
   this->clone = this->stub->create_working_copy();
   assert(this->clone);
   this->_load_impl();
}
void FormDialogWorkingCopyBase::save() {
   if (!this->stub)
      return;
   this->clone = nullptr;
   //
   auto& editor = DovahKitCore::get();
   emit editor.formWorkingCopyCommitImminent(this->stub);
   emit editor.formModificationImminent(this->stub);
   this->stub->set_edited(true);
   this->stub->commit_working_copy();
   this->_save_impl();
   emit editor.formWorkingCopyCommitComplete(this->stub);
   emit editor.formModified(this->stub);
}
#pragma endregion