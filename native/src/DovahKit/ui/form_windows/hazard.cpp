#include "./hazard.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogHazard::FormDialogHazard(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.light->setAllowedFormType(dovah::form_type::light);
   this->ui.sound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.imagespaceMod->setAllowedFormType(dovah::form_type::imagespace_modifier);
   this->ui.impactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
   this->ui.spell->setAllowedFormTypes({ dovah::form_type::spell, dovah::form_type::enchantment });

   ui::set_range<int32_t>(this->ui.maxCount); // TODO: Create a QSpinBox that can hold uint32_t; QSpinBox itself is capped to int32_t (int).
   ui::set_unsigned_range<float>(this->ui.lifetime);
   ui::set_unsigned_range<float>(this->ui.interval);
   ui::set_unsigned_range<float>(this->ui.radius);
   ui::set_unsigned_range<float>(this->ui.imagespaceRadius);

   this->load(); // this creates the working copy.
}
void FormDialogHazard::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.radius, working.radius);
   ui::bind(this->ui.light, working.light, working);
   ui::bind(this->ui.sound, working.sound, working);
   ui::bind(this->ui.spell, working.spell, working);
   ui::bind(this->ui.imagespaceMod, working.imagespace_modifier, working);
   ui::bind(this->ui.imagespaceRadius, working.imagespace_radius);
   ui::bind(this->ui.impactDataSet, working.impact_data_set, working);
   ui::bind(this->ui.interval, working.target_interval);
   ui::bind(this->ui.lifetime, working.lifetime);
   ui::bind(this->ui.maxCount, working.limit);
   ui::bind(this->ui.flagPlayerOnly,          working.hazard_flags, loaded_form_type::hazard_flag::only_affects_player);
   ui::bind(this->ui.flagInheritDuration,     working.hazard_flags, loaded_form_type::hazard_flag::inherit_duration_from_source_spell);
   ui::bind(this->ui.flagInheritRadius,       working.hazard_flags, loaded_form_type::hazard_flag::inherit_radius_from_source_spell);
   ui::bind(this->ui.flagAlignToImpactNormal, working.hazard_flags, loaded_form_type::hazard_flag::align_to_impact_normal);
   ui::bind(this->ui.flagDropToGround,        working.hazard_flags, loaded_form_type::hazard_flag::drop_to_ground);
}
void FormDialogHazard::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);
}