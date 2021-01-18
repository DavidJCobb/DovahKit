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
   {
      auto* widget = this->ui.forms;
      auto& list   = this->form->contents;
      widget->clear();
      widget->reserve(list.size());
      for (auto& ref : list)
         widget->addStub(ref.get_form_stub());
   }
}
void FormDialogFormList::_save_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   //
   auto&  list  = this->form->contents;
   auto   stubs = this->ui.forms->stubs();
   size_t i     = 0;
   size_t size  = stubs.size();
   if (list.size() < size)
      list.resize(size);
   for (; i < size; ++i)
      list[i].set(*this->stub, stubs[i]);
   //
   // Delete excess elements, if any were removed:
   //
   auto s = list.size();
   if (s != size) {
      for (; i < s; ++i)
         list[i].set(*this->stub, nullptr);
      list.resize(size);
   }
}