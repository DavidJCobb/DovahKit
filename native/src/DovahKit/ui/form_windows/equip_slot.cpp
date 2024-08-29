#include "./equip_slot.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogEquipSlot::FormDialogEquipSlot(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->load(); // this creates the working copy.
}
void FormDialogEquipSlot::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.parents->pullStubs(working.parent_slots);
   ui::bind(this->ui.flagUseAllParents, working.local_flags, loaded_form_type::local_flag::use_all_parents);
}
void FormDialogEquipSlot::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& working = *this->form;
   this->ui.parents->commitStubs(working.parent_slots, working);
}