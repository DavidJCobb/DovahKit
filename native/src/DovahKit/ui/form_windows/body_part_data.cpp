#include "./body_part_data.h"
#include "editor/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/show_parent_scoped_modal.h"
#include "./body_part_data/BodyPartDataPartsModel.h"
#include "./body_part_data/FormSubdialogBodyPartDataBodyPart.h"
#include "./body_part_data/SkeletonBonesModel.h"

FormDialogBodyPartData::FormDialogBodyPartData(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.ragdollData->setAllowedFormType(dovah::form_type::ragdoll);
   this->ui.ragdollData->setAllowNone(true);

   this->models.bones = new SkeletonBonesModel(this);
   this->models.parts = new BodyPartDataPartsModel(this);
   this->models.parts->setBonesModel(this->models.bones);

   QObject::connect(this->ui.skeleton, &DKGameFilePicker::valueChanged, this, [this](ui::types::game_file_path path) {
      this->models.bones->resetFromSkeleton(path.lexically_relative("Data\\Meshes\\").to_string().toStdString());
   });

   {
      auto* view = this->ui.parts;
      view->setModel(this->models.parts);

      QObject::connect(this->ui.buttonPartNew,    &QPushButton::pressed, this, &FormDialogBodyPartData::create_part);
      QObject::connect(this->ui.buttonPartEdit,   &QPushButton::pressed, this, &FormDialogBodyPartData::edit_part);
      QObject::connect(this->ui.buttonPartDelete, &QPushButton::pressed, this, &FormDialogBodyPartData::delete_selected_part);
   }

   this->load(); // this creates the working copy.
}
void FormDialogBodyPartData::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {
      ui::types::game_file_path path("Data\\Meshes");
      path.append(working.model.model_path.c_str());
      this->ui.skeleton->setValue(path);
      this->models.bones->resetFromSkeleton(path.lexically_relative("Data\\Meshes\\").to_string().toStdString());
   }
   ui::bind(this->ui.ragdollData, working.ragdoll, working);

   this->models.parts->importData(working);
}
void FormDialogBodyPartData::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   {
      auto path = this->ui.skeleton->value();
      path = path.lexically_relative("Data\\Meshes\\");
      working.model.model_path = path.to_string().toStdString();
   }
   this->models.parts->exportData(working);
}

void FormDialogBodyPartData::create_part() {
   auto* modal = new FormSubdialogBodyPartDataBodyPart(this);
   modal->setBonesModel(this->models.bones);
   QObject::connect(modal, &QDialog::accepted, this, [this, modal]() {
      auto row = this->models.parts->rowCount();
      if (this->models.parts->insertRow(row)) {
         this->models.parts->replaceBodyPart(row, modal->value());
      }
   });
   QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
   ui::show_parent_scoped_modal(*modal, this);
}
void FormDialogBodyPartData::edit_part() {
   auto* sm  = this->ui.parts->selectionModel();
   auto  sel = sm->selection();
   if (sel.empty())
      return;

   const auto  row = sel[0].top();
   const auto* data_ptr = this->models.parts->bodyPart(row);
   if (!data_ptr)
      return;

   auto* modal = new FormSubdialogBodyPartDataBodyPart(this);
   modal->setBonesModel(this->models.bones);
   modal->setValue(*data_ptr);
   QObject::connect(modal, &QDialog::accepted, this, [this, modal, row]() {
      this->models.parts->replaceBodyPart(row, modal->value());
   });
   QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
   ui::show_parent_scoped_modal(*modal, this);
}
void FormDialogBodyPartData::delete_selected_part() {
   auto* sm  = this->ui.parts->selectionModel();
   auto  sel = sm->selection();
   if (sel.empty())
      return;
   this->models.parts->removeRow(sel[0].top());
}