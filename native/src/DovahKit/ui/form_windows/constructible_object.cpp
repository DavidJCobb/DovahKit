#include "./constructible_object.h"
#include "dovah/core.h"
#include "dovah/data/all_item_form_types.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogConstructibleObject::FormDialogConstructibleObject(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   for (auto ft : dovah::all_item_form_types)
      this->ui.resultForm->addAllowedFormType(ft);
   ui::set_range<decltype(decltype(loaded_form_type::result)::count)>(this->ui.resultCount);
   this->ui.workbenchKeyword->setAllowedFormType(dovah::form_type::keyword);
   this->ui.requiredItems->setAllowsPseudoItems(false);

   this->load(); // this creates the working copy.
}
void FormDialogConstructibleObject::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.resultForm, working.result.form, working);
   ui::bind(this->ui.resultCount, working.result.count);
   ui::bind(this->ui.workbenchKeyword, working.workbench_keyword, working);
   this->ui.requiredItems->initializeFrom(working.inventory);
   this->ui.conditions->importFrom(working, working.conditions);
}
void FormDialogConstructibleObject::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   this->ui.requiredItems->commitTo(working.inventory, working);
   this->ui.conditions->exportTo(working, working.conditions);
}