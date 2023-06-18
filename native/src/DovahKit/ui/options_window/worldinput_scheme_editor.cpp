#include "./worldinput_scheme_editor.h"
#include "editor/subsystems/worldinput2/bind_tree/tree.h"

WorldinputSchemeEditDialog::WorldinputSchemeEditDialog(input_device_type idt, QWidget* parent) : QDialog(parent), device_type(idt) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, &QDialog::accept);

   this->ui.name->setMaxLength(control_scheme_type::max_name_length);

   // TODO: Treeview
   // TODO: Treeview model (this->_model)
   // TODO: Node edit buttons
}

void WorldinputSchemeEditDialog::initializeFrom(const control_scheme_type& src) {
   this->ui.name->setText(src.name);

   // TODO: Node tree
}
void WorldinputSchemeEditDialog::overwrite(control_scheme_type& dst) const {
   dst.name = this->ui.name->text();

   // TODO: Node tree
}