#include "./worldinput_scheme_editor.h"
#include "editor/subsystems/worldinput2/bind_tree/tree.h"
#include "ui/models/worldinput/DKWorldinputControlSchemeModel.h"
#include "widgets/DKHeaderView.h"

WorldinputSchemeEditDialog::WorldinputSchemeEditDialog(input_device_type idt, QWidget* parent) : QDialog(parent), device_type(idt) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, &QDialog::accept);

   this->ui.name->setMaxLength(control_scheme_type::max_name_length);

   {
      auto* treeview = this->ui.nodeTree;
      treeview->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
      treeview->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      treeview->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
      treeview->expandAll();

      // QHeaderView sucks, and is bad, so replace it with this
      auto* header = new DKHeaderView(Qt::Orientation::Horizontal, treeview);
      treeview->setHeader(header);
      //
      header->setFlexResizeEnabled(true);

      auto* model = this->_model = new DKWorldinputControlSchemeModel(treeview);
      treeview->setModel(model);
   }
   // TODO: Node edit buttons
}

void WorldinputSchemeEditDialog::initializeFrom(const control_scheme_type& src) {
   this->ui.name->setText(src.name);

   this->_model->overwriteFromSource(src);
   this->ui.nodeTree->expandAll();
}
void WorldinputSchemeEditDialog::overwrite(control_scheme_type& dst) const {
   dst.name = this->ui.name->text();

   this->_model->overwriteDestination(dst);
}