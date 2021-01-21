#include "formlist.h"
#include "_base_cpp.h"

FormDialogFormList::FormDialogFormList(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogFormList, dovah::loaded_forms::FormList>(*this, stub);
   //
   QObject::connect(this->ui.buttonMoveUp,   &QPushButton::clicked, this, [this]() { this->ui.forms->moveSelected(-1); });
   QObject::connect(this->ui.buttonMoveDown, &QPushButton::clicked, this, [this]() { this->ui.forms->moveSelected(1); });
   QObject::connect(this->ui.buttonRemove, &QPushButton::clicked, this, [this]() { this->ui.forms->removeSelected(); });
   //
   this->ui.forms->setAcceptDrops(true);
   //
   this->load();
}
void FormDialogFormList::_load_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.forms->import(this->form->contents);
}
void FormDialogFormList::_save_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   this->ui.forms->commit(this->form->contents, *this->form);
}