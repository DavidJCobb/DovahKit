#include "./leveled_item.h"
#include "./leveled_lists/LeveledListEditDialogHelpers.h"

FormDialogLeveledItem::FormDialogLeveledItem(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   LeveledListEditDialogHelpers::setup(*this);

   this->load(); // this creates the working copy.
}

void FormDialogLeveledItem::_overwrite_selected_leveled_object() {
   #if _DEBUG
      auto* _widget_what_triggered_this_here_call = sender();
   #endif
   LeveledListEditDialogHelpers::write_ui_to_model(*this);
}

void FormDialogLeveledItem::_load_impl() {
   LeveledListEditDialogHelpers::load(*this);
}
void FormDialogLeveledItem::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   LeveledListEditDialogHelpers::save(*this);
}

/*virtual*/ bool FormDialogLeveledItem::eventFilter(QObject* watched, QEvent* event) /*override*/ {
   return LeveledListEditDialogHelpers::event_filter(*this, watched, event);
}