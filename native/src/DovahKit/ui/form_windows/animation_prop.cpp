#include "./animation_prop.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogAnimationProp::FormDialogAnimationProp(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.unloadEvent->setMaxLength(loaded_form_type::max_unload_event_name_length);

   this->load(); // this creates the working copy.
}
void FormDialogAnimationProp::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.unloadEvent, working.unload_event);
}
void FormDialogAnimationProp::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   this->ui.model->commitTo(working.model, working);
}