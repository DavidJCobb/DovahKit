#include "./idle_marker.h"
#include "ui/utils/bind.h"

FormDialogIdleMarker::FormDialogIdleMarker(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->load(); // this creates the working copy.
}
void FormDialogIdleMarker::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.marker->initializeFrom(working.model);
   ui::bind(this->ui.flagChild, this->record_flags(), loaded_form_type::form_flag::child_can_use);
   ui::bind(this->ui.flagIgnoredBySandbox, working.data.flags, loaded_form_type::flag::ignored_by_sandbox);
   this->ui.idles->importData(working.data);
}
void FormDialogIdleMarker::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->ui.marker->commitTo(working.model, working);
   this->ui.idles->exportData(working.data, working);
}