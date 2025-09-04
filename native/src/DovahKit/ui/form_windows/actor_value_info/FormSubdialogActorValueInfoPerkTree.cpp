#include "./FormSubdialogActorValueInfoPerkTree.h"
#include "./SkillTreeVisualEditor.h"

FormSubdialogActorValueInfoPerkTree::FormSubdialogActorValueInfoPerkTree(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   auto* editor = this->_widgets.editor = new SkillTreeVisualEditor(this);
   this->ui.scrollbox->setWidget(editor);
   editor->setContainingScrollArea(this->ui.scrollbox);

   #pragma region Zoom
      QObject::connect(this->ui.buttonZoomIn, &QPushButton::clicked, this, [this]() {
         auto* widget = this->_widgets.editor;
         auto  zoom   = widget->zoom();
         if (zoom >= 10.0F) {
            return;
         }
         zoom += 0.10F;
         widget->setZoom(zoom);
         this->ui.buttonZoomIn->setEnabled(zoom < 10.0F);
         this->ui.buttonZoomOut->setEnabled(zoom > 0.1F);
      });
      QObject::connect(this->ui.buttonZoomOut, &QPushButton::clicked, this, [this]() {
         auto* widget = this->_widgets.editor;
         auto  zoom   = widget->zoom();
         if (zoom <= 0.1F) {
            return;
         }
         zoom -= 0.1F;
         widget->setZoom(zoom);
         this->ui.buttonZoomIn->setEnabled(zoom < 10.0F);
         this->ui.buttonZoomOut->setEnabled(zoom > 0.1F);
      });
   #pragma endregion

   this->ui.currentNodePerk->setAllowedFormType(dovah::form_type::perk);
   this->ui.currentNodeSkill->setAllowedFormType(dovah::form_type::actor_value_info);

   this->_pull_perk_node();
   QObject::connect(
      editor,
      &SkillTreeVisualEditor::selectionChanged,
      this,
      [this](SkillTreeVisualEditor::optional_node_id selected, SkillTreeVisualEditor::optional_node_id prior) {
         if (prior.has_value())
            this->_push_perk_node(prior);
         this->_pull_perk_node(selected);
      }
   );

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->_push_perk_node();
      this->_widgets.editor->exportData();
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->_widgets.editor->clear();
      this->reject();
   });

   static_assert(true, "TODO: Start testing before we worry about this, BUT: we need a way to connect and disconnect two perk nodes. Current UI doesn't have that.");
}

void FormSubdialogActorValueInfoPerkTree::setOwningActorValue(dovah::form_stub* av) {
   auto* widget = this->_widgets.editor;
   widget->setCurrentActorValue(av);
   widget->importData();
}

void FormSubdialogActorValueInfoPerkTree::_push_perk_node(std::optional<uint32_t> node_id) {
   if (!node_id.has_value()) {
      node_id = this->_widgets.editor->selectedNodeID();
      if (!node_id.has_value()) {
         return;
      }
   }
   SkillTreeVisualEditor::PerkNodeData node;
   node.perk = this->ui.currentNodePerk->formStub();
   node.skill = this->ui.currentNodeSkill->formStub();
   node.parent_required = this->ui.currentNodeRequiresParent->isChecked();
   node.position = {
      .grid = {
         .x = (uint32_t)this->ui.currentNodeGridX->value(),
         .y = (uint32_t)this->ui.currentNodeGridY->value()
      },
      .offset = {
         .x = (float)this->ui.currentNodeOffsetX->value(),
         .y = (float)this->ui.currentNodeOffsetY->value()
      }
   };
   this->_widgets.editor->setNodeData(node_id.value(), node);
}
void FormSubdialogActorValueInfoPerkTree::_pull_perk_node(std::optional<uint32_t> node_id) {
   if (!node_id.has_value()) {
      node_id = this->_widgets.editor->selectedNodeID();
      if (!node_id.has_value()) {
         this->ui.groupboxCurrentNode->setEnabled(false);
         return;
      }
   }
   const auto* node = this->_widgets.editor->nodeData(node_id.value());
   if (!node) {
      this->ui.groupboxCurrentNode->setEnabled(false);
      return;
   }
   this->ui.groupboxCurrentNode->setEnabled(true);
   this->ui.currentNodePerk->setFormStub(node->perk);
   this->ui.currentNodeSkill->setFormStub(node->skill);
   this->ui.currentNodeRequiresParent->setChecked(node->parent_required);
   this->ui.currentNodeGridX->setValue(node->position.grid.x);
   this->ui.currentNodeGridY->setValue(node->position.grid.y);
   this->ui.currentNodeOffsetX->setValue(node->position.offset.x);
   this->ui.currentNodeOffsetY->setValue(node->position.offset.y);
}