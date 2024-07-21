#include "./outfit.h"
#include "editor/subsystems/gui_adjust/core.h"

FormDialogOutfit::FormDialogOutfit(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   initialize(stub);
   dovahkit::subsystems::gui_adjust::core::get_or_create().registerWidgets(*this, {
      { "OutfitFL", this->ui.forms },
   });
   this->ui.forms->setAllowedFormTypes({ dovah::form_type::armor, dovah::form_type::leveled_item });
   
   this->load();
}
void FormDialogOutfit::_load_impl() {
   auto& editor = DovahKitCore::get();
   
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.forms->pullStubs(this->form->contents);
}
void FormDialogOutfit::_save_impl() {
   auto& editor = DovahKitCore::get();
   
   this->stub->editorID = this->ui.editorID->text().toStdString();
   this->ui.forms->commitStubs(this->form->contents, *this->form);
}