#include "./static.h"
#include "dovah/data/game.h"
#include "ui/utils/bind.h"

#include "./static/FormSubdialogStaticLODMeshes.h"

FormDialogStatic::FormDialogStatic(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.directionalMaterial->setAllowedFormType(dovah::form_type::material_object);

   QObject::connect(this->ui.flagHasDistantLOD, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.flagHasHighDetailLODTexture->setEnabled(checked);
      this->ui.flagAddOnLODObject->setEnabled(checked);
      this->ui.buttonSpecifyLODMeshes->setEnabled(checked);
   });

   QObject::connect(this->ui.buttonSpecifyLODMeshes, &QPushButton::clicked, this, [this]() {
      auto* dialog = new FormSubdialogStaticLODMeshes(this);
      dialog->setPaths(this->form->distant_lod_paths);
      
      auto result = dialog->exec();
      if (result == QDialog::Accepted) {
         this->form->distant_lod_paths = dialog->getPaths();
      }
      dialog->deleteLater();
   });

   this->load(); // this creates the working copy.
}
void FormDialogStatic::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {  // Flags
      auto& record_flags = this->record_flags();

      ui::bind_inverse(this->ui.flagOnLocalMap, record_flags, loaded_form_type::form_flag::hide_from_local_map);
      ui::bind(this->ui.flagObstacle,        record_flags, loaded_form_type::form_flag::obstacle);
      ui::bind(this->ui.flagIsMarker,        record_flags, loaded_form_type::form_flag::is_marker);
      //
      ui::bind(this->ui.flagHasTreeLOD,      record_flags, loaded_form_type::form_flag::has_tree_lod);
      ui::bind(this->ui.flagNeverFades,      record_flags, loaded_form_type::form_flag::never_fades);
      ui::bind(this->ui.flagHasDistantLOD,   record_flags, loaded_form_type::form_flag::has_distant_lod);
      //
      ui::bind(this->ui.flagHasHighDetailLODTexture, record_flags, loaded_form_type::form_flag::uses_hd_lod_texture);
      ui::bind(this->ui.flagAddOnLODObject, record_flags, loaded_form_type::form_flag::addon_lod_object);
      //
      ui::bind(this->ui.flagShowInWorldMap, record_flags, loaded_form_type::form_flag::show_in_world_map);
      ui::bind(this->ui.flagHasCurrents,    record_flags, loaded_form_type::form_flag::has_currents);
   }
   this->ui.model->initializeFrom(working.model);

   ui::bind(this->ui.directionalMaterial,      working.directional_material.material_object, working);
   ui::bind(this->ui.directionalMaterialAngle, working.directional_material.max_angle);
   ui::bind(this->ui.directionalMaterialIsSnow, working.directional_material.flags, loaded_form_type::directional_material_data::flag::is_snow);
   if (DovahKitCore::get().get_current_game() != dovah::game::skyrim_special) {
      this->ui.directionalMaterialIsSnow->setVisible(false);
   }

   ui::bind(this->ui.navmeshGeneration, this->record_flags());

   //this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogStatic::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   this->ui.model->commitTo(working.model, working);

   //this->ui.scriptListPane->commit();
}