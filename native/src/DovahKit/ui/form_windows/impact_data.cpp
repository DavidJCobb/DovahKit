#include "./impact_data.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"

using decal_flag = dovah::loaded_forms::components::decal_data::flag;

FormDialogImpactData::FormDialogImpactData(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.textureSetMain->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.textureSetSecondary->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.effectHazard->setAllowedFormType(dovah::form_type::hazard);
   this->ui.sound1->setAllowedFormTypes({ dovah::form_type::sound, dovah::form_type::sound_descriptor });
   this->ui.sound2->setAllowedFormTypes({ dovah::form_type::sound, dovah::form_type::sound_descriptor });
   {
      auto* widget = this->ui.detectionSoundLevel;
      widget->clear();
      widget->addItem(tr("Silent"), (int)dovah::detection_loudness::silent);
      widget->addItem(tr("Normal"), (int)dovah::detection_loudness::normal);
      widget->addItem(tr("Loud"), (int)dovah::detection_loudness::loud);
      widget->addItem(tr("Very Loud"), (int)dovah::detection_loudness::very_loud);
   }
   {
      auto* widget = this->ui.effectOrientation;
      widget->clear();
      widget->addItem(tr("Projectile Reflection"), (int)loaded_form_type::effect_orientation::projectile_reflection);
      widget->addItem(tr("Projectile Vector"), (int)loaded_form_type::effect_orientation::projectile_vector);
      widget->addItem(tr("Surface Normal"), (int)loaded_form_type::effect_orientation::surface_normal);
      widget->model()->sort(0);
   }
   {
      using enumeration = loaded_form_type::impact_result_type;
      auto* widget = this->ui.effectImpactResult;
      widget->clear();
      widget->addItem(tr("Default"), (int)enumeration::default_result);
      widget->addItem(tr("Destroy"), (int)enumeration::destroy);
      widget->addItem(tr("Bounce"), (int)enumeration::bounce);
      widget->addItem(tr("Impale"), (int)enumeration::impale);
      widget->addItem(tr("Stick"), (int)enumeration::stick);
      widget->model()->sort(0);
   }

   this->load(); // this creates the working copy.
}
void FormDialogImpactData::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.textureSetMain, working.decal.texture_sets.primary, working);
   ui::bind(this->ui.textureSetSecondary, working.decal.texture_sets.secondary, working);
   ui::bind(this->ui.radius, working.placement_radius);
   ui::bind(this->ui.angleThreshold, working.angle_threshold);

   ui::bind_inverse(this->ui.decalData, working.decal.enabled);
   if (auto* src = working.decal_data) {
      this->ui.widthMin->setValue(src->width.min);
      this->ui.widthMax->setValue(src->width.max);
      this->ui.heightMin->setValue(src->height.min);
      this->ui.heightMax->setValue(src->height.max);
      this->ui.color->setColor(QColor(src->color.r, src->color.g, src->color.b));
      this->ui.depth->setValue(src->depth);
      this->ui.shininess->setValue(src->shininess);
      this->ui.flagDecalAlphaBlend->setChecked(src->flags & decal_flag::alpha_blending);
      this->ui.flagDecalAlphaTest->setChecked(src->flags & decal_flag::alpha_testing);
      this->ui.flagParallax->setChecked(src->flags & decal_flag::parallax);
      this->ui.parallaxPasses->setValue(src->parallax.passes);
      this->ui.parallaxScale->setValue(src->parallax.scale);
      this->ui.flag4Subtex->setChecked(!(src->flags & decal_flag::no_subtextures));
   }

   this->ui.effectModel->initializeFrom(working.model);
   ui::bind(this->ui.effectDuration, working.effect.duration);
   ui::bind(this->ui.effectOrientation, working.effect.orientation);
   ui::bind(this->ui.effectHazard, working.hazard, working);
   ui::bind(this->ui.effectImpactResult, working.impact_result);

   ui::bind(this->ui.sound1, working.sounds[0], working);
   ui::bind(this->ui.sound2, working.sounds[1], working);
   ui::bind(this->ui.detectionSoundLevel, working.loudness);
}
void FormDialogImpactData::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   if (working.decal.enabled) {
      auto* data = working.decal_data;
      if (!data) {
         data = working.decal_data = new dovah::loaded_forms::components::decal_data;
      }
      data->width = {
         .min = (float)this->ui.widthMin->value(),
         .max = (float)this->ui.widthMax->value(),
      };
      data->height = {
         .min = (float)this->ui.heightMin->value(),
         .max = (float)this->ui.heightMax->value(),
      };
      {
         auto c = this->ui.color->color();
         data->color.r = c.red();
         data->color.g = c.green();
         data->color.b = c.blue();
      }
      data->depth = this->ui.depth->value();
      data->shininess = this->ui.shininess->value();
      data->parallax = {
         .scale  = (float)this->ui.parallaxScale->value(),
         .passes = (uint8_t)this->ui.parallaxPasses->value(),
      };

      cobb::edit_bit(data->flags, decal_flag::alpha_blending, this->ui.flagDecalAlphaBlend->isChecked());
      cobb::edit_bit(data->flags, decal_flag::alpha_testing,  this->ui.flagDecalAlphaTest->isChecked());
      cobb::edit_bit(data->flags, decal_flag::parallax,       this->ui.flagParallax->isChecked());
      cobb::edit_bit(data->flags, decal_flag::no_subtextures, !this->ui.flag4Subtex->isChecked());
   } else {
      if (auto* data = working.decal_data) {
         working.decal_data = nullptr;
         delete data;
      }
   }

   this->ui.effectModel->commitTo(working.model, working);
}