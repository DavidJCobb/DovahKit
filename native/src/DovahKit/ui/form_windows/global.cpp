#include "./global.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogGlobal::FormDialogGlobal(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::set_range<float>(this->ui.value);
   this->ui.type->setItemData(0, (int)loaded_form_type::value_type::int16);
   this->ui.type->setItemData(1, (int)loaded_form_type::value_type::int32);
   this->ui.type->setItemData(2, (int)loaded_form_type::value_type::float32);

   this->load(); // this creates the working copy.
}
void FormDialogGlobal::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.flagConstant, this->record_flags(), loaded_form_type::form_flag::constant);
   ui::bind(this->ui.type,  working.value_type);
   ui::bind(this->ui.value, working.value);

   //this->ui.scriptListPane->setFormWorkingCopy(&working); // TODO
}
void FormDialogGlobal::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   //this->ui.scriptListPane->commit(); // TODO
}