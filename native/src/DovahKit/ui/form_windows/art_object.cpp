#include "./art_object.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogArtObject::FormDialogArtObject(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   {
      auto* widget = this->ui.type;
      widget->clear();
      widget->addItem(tr("Magic Casting"), (int)loaded_form_type::art_type::magic_casting);
      widget->addItem(tr("Magic Hit Effect"), (int)loaded_form_type::art_type::magic_hit_effect);
      widget->addItem(tr("Enchantment Effect"), (int)loaded_form_type::art_type::enchantment_effect);
   }

   this->load(); // this creates the working copy.
}
void FormDialogArtObject::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.type, working.type);
   this->ui.model->initializeFrom(working.model);
}
void FormDialogArtObject::_save_impl() {
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