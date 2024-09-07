#include "./dual_cast_data.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogDualCastData::FormDialogDualCastData(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.hitEffectArt->setAllowedFormType(dovah::form_type::art_object);
   this->ui.effectShader->setAllowedFormType(dovah::form_type::effect_shader);
   this->ui.projectile->setAllowedFormType(dovah::form_type::projectile);
   this->ui.explosion->setAllowedFormType(dovah::form_type::explosion);
   this->ui.impactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);

   this->load(); // this creates the working copy.
}
void FormDialogDualCastData::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());

   ui::bind(this->ui.hitEffectArt,  working.hit_effect_art, working);
   ui::bind(this->ui.effectShader,  working.effect_shader, working);
   ui::bind(this->ui.projectile,    working.projectile, working);
   ui::bind(this->ui.explosion,     working.explosion, working);
   ui::bind(this->ui.impactDataSet, working.impact_data_set, working);
   //
   ui::bind(this->ui.inheritScaleE,   working.inherit_scale.explosion);
   ui::bind(this->ui.inheritScaleP,   working.inherit_scale.projecile);
   ui::bind(this->ui.inheritScaleHEA, working.inherit_scale.hit_effect_art);
}
void FormDialogDualCastData::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
}