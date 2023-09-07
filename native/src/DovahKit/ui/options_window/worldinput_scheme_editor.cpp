#include "./worldinput_scheme_editor.h"
#include "editor/subsystems/worldinput/control_scheme.h"
#include "ui/models/worldinput/DKWorldinputControlSchemeModel.h"
#include "widgets/DKHeaderView.h"

#include "./worldinput_bind_editor.h"
#include "./worldinput_condition_editor.h"
#include "./worldinput_modifier_editor.h"

namespace {
   using dovahkit::subsystems::worldinput::control_scheme_action;
   using dovahkit::subsystems::worldinput::control_scheme_condition_node;
   using dovahkit::subsystems::worldinput::control_scheme_modifier;
}

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
   QObject::connect(this->ui.buttonNewBind, &QPushButton::clicked, this, [this]() {
      auto after_qmi = this->_getFirstSelectedNode();
      auto inserted  = this->_model->insertAfter(after_qmi, control_scheme_action{
         .name = tr("New Action", "default new action name")
      });
      if (inserted.isValid()) {
         (this->ui.nodeTree->selectionModel())->select(inserted, QItemSelectionModel::ClearAndSelect);
      }
   });
   QObject::connect(this->ui.buttonNewEditorModeNode, &QPushButton::clicked, this, [this]() {
      auto after_qmi = this->_getFirstSelectedNode();
      auto inserted  = this->_model->insertAfter(after_qmi, control_scheme_condition_node{
         .name = tr("New Condition", "default new condition name")
      });
      if (inserted.isValid()) {
         (this->ui.nodeTree->selectionModel())->select(inserted, QItemSelectionModel::ClearAndSelect);
      }
   });
   QObject::connect(this->ui.buttonNewModifier, &QPushButton::clicked, this, [this]() {
      auto after_qmi = this->_getFirstSelectedNode();
      auto inserted  = this->_model->insertAfter(after_qmi, control_scheme_modifier{
         .name = tr("New Modifier", "default new modifier name")
      });
      if (inserted.isValid()) {
         (this->ui.nodeTree->selectionModel())->select(inserted, QItemSelectionModel::ClearAndSelect);
      }
   });
   QObject::connect(this->ui.buttonEditNode, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_getFirstSelectedNode();
      if (!qmi.isValid())
         return;
      auto data = this->_model->infoFor(qmi);
      if (std::holds_alternative<std::monostate>(data))
         return;

      bool modified = false;
      if (std::holds_alternative<control_scheme_action>(data)) {
         auto& casted = std::get<control_scheme_action>(data);
         auto* editor = new WorldinputBindEditDialog(this->device_type, this);
         editor->initializeFrom(casted);
         auto  result = editor->exec();
         if (result == QDialog::DialogCode::Accepted) {
            modified = true;
            editor->overwrite(casted);
         }
      } else if (std::holds_alternative<control_scheme_modifier>(data)) {
         auto& casted = std::get<control_scheme_modifier>(data);
         auto* editor = new WorldinputModifierEditDialog(this->device_type, this);
         editor->initializeFrom(casted);
         auto  result = editor->exec();
         if (result == QDialog::DialogCode::Accepted) {
            modified = true;
            editor->overwrite(casted);
         }
      } else if (std::holds_alternative<control_scheme_condition_node>(data)) {
         auto& casted = std::get<control_scheme_condition_node>(data);
         auto* editor = new WorldinputConditionEditDialog(this);
         editor->initializeFrom(casted);
         auto  result = editor->exec();
         if (result == QDialog::DialogCode::Accepted) {
            modified = true;
            editor->overwrite(casted);
         }
      }

      if (modified) {
         this->_model->replaceInfoFor(qmi, data);
         if (auto* sm = this->ui.nodeTree->selectionModel()) {
            //
            // If there's a multiple-selection, deselect all but the QMI we actually edited.
            //
            sm->select(qmi, QItemSelectionModel::ClearAndSelect);
         }
      }
   });
   QObject::connect(this->ui.buttonMoveNodeUp, &QPushButton::clicked, this, [this]() {
      this->_model->moveItems(this->_getSelection(), -1);
   });
   QObject::connect(this->ui.buttonMoveNodeDown, &QPushButton::clicked, this, [this]() {
      this->_model->moveItems(this->_getSelection(), 1);
   });
   QObject::connect(this->ui.buttonDeleteNode, &QPushButton::clicked, this, [this]() {
      this->_model->deleteItems(this->_getSelection().indexes());
   });
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
WorldinputSchemeEditDialog::control_scheme_type WorldinputSchemeEditDialog::retrieve() const {
   control_scheme_type out(this->device_type);
   out.name = this->ui.name->text();

   this->_model->overwriteDestination(out);

   return out;
}

QModelIndex WorldinputSchemeEditDialog::_getFirstSelectedNode() {
   auto* sm = this->ui.nodeTree->selectionModel();
   if (!sm)
      return {};
   return sm->currentIndex();
}
const QItemSelection WorldinputSchemeEditDialog::_getSelection() {
   auto* sm = this->ui.nodeTree->selectionModel();
   if (!sm)
      return {};
   return sm->selection();
}