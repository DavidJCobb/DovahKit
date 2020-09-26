#include "cell.h"
#include "_base_cpp.h"
#include "../../helpers/bitwise.h"
#include "../../helpers/qt/spinbox.h"
#include "../../dovah/forms/factories/hardcoded.h"
#include "../../dovah/forms/components/extra_data/cell_acoustic_space.h"
#include "../../dovah/forms/components/extra_data/cell_climate.h"
#include "../../dovah/forms/components/extra_data/cell_imagespace.h"
#include "../../dovah/forms/components/extra_data/cell_music_override.h"
#include "../../dovah/forms/components/extra_data/cell_water_type.h"
#include "../../dovah/forms/components/extra_data/encounter_zone.h"
#include "../../dovah/forms/components/extra_data/interior_lock_list.h"
#include "../../dovah/forms/components/extra_data/location.h"
#include "../../dovah/forms/components/extra_data/ownership.h"
#include "../../dovah/forms/components/extra_data/rank.h"
#include "../../dovah/forms/components/extra_data/water_data.h"
#include "../../dovah/forms/components/extra_data/water_environment_map.h"

namespace {
   using form_flag = dovah::loaded_forms::Cell::form_flag;
   using cell_flag = dovah::loaded_forms::Cell::cell_flag;
   using land_flag = dovah::loaded_forms::Cell::land_flag;
   using inherit_flag = dovah::loaded_forms::components::interior_lighting::inherit_flag;

   using extra_data_type = dovah::loaded_forms::components::extra_data_type;
   namespace extra {
      using namespace dovah::loaded_forms::components::extra;
   }

   template<class ec, extra_data_type et> void _load_extra_formID(FormsOfTypeCombobox* widget, dovah::loaded_forms::components::extra_data_list& extra) {
      if (auto* data = extra.lookup<ec>(et)) {
         widget->setFormByID(data->formID);
      } else {
         widget->setFormByID(0);
      }
   }

   QColor _form_color_to_q(const dovah::loaded_forms::color_t c) {
      return QColor(c.r, c.g, c.b, 255);
   }
}
FormDialogCell::FormDialogCell(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogCell, dovah::loaded_forms::Cell>(*this, stub);
   //
   cobb::qt::remove_spinbox_bounds(this->ui.lightingDirectionalFade);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingDirectionalRotationXY);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingDirectionalRotationZ);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFadeDistanceStart);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFadeDistanceEnd);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFogDistanceNear);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFogDistanceFar);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFogClipDistance);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFogMax);
   cobb::qt::remove_spinbox_bounds(this->ui.lightingFogPower);
   cobb::qt::remove_spinbox_bounds(this->ui.waterHeight);
   cobb::qt::remove_spinbox_bounds(this->ui.waterAngularVelocityX);
   cobb::qt::remove_spinbox_bounds(this->ui.waterAngularVelocityY);
   cobb::qt::remove_spinbox_bounds(this->ui.waterAngularVelocityZ);
   cobb::qt::remove_spinbox_bounds(this->ui.waterLinearVelocityX);
   cobb::qt::remove_spinbox_bounds(this->ui.waterLinearVelocityY);
   cobb::qt::remove_spinbox_bounds(this->ui.waterLinearVelocityZ);
   //
   this->ui.tabs->setCurrentIndex(0);
   //
   this->ui.location->addFormType(dovah::form_type::location);
   this->ui.acousticSpace->addFormType(dovah::form_type::acoustic_space);
   this->ui.imagespace->addFormType(dovah::form_type::imagespace);
   this->ui.musicType->addFormType(dovah::form_type::music_type);
   this->ui.waterType->addFormType(dovah::form_type::water_type);
   this->ui.location->setAllowNone(true);
   this->ui.acousticSpace->setAllowNone(true);
   this->ui.imagespace->setAllowNone(true);
   this->ui.imagespace->setNoneLabel(tr("DEFAULT", "cell imagespace"));
   this->ui.musicType->setAllowNone(true);
   this->ui.musicType->setAllowUndefined(true);
   this->ui.musicType->setUndefinedLabel(tr("DEFAULT", "cell music"));
   this->ui.waterType->setDefaultFormID(dovah::hardcoded_form_ids::DefaultWater);
   this->ui.location->populate();
   this->ui.acousticSpace->populate();
   this->ui.imagespace->populate();
   this->ui.musicType->populate();
   this->ui.waterType->populate();
   //
   this->ui.lightingTemplate->addFormType(dovah::form_type::lighting_template);
   this->ui.lightingTemplate->setAllowNone(true);
   this->ui.skyRegion->addFormType(dovah::form_type::region);
   this->ui.skyRegion->setAllowNone(true);
   this->ui.lightingTemplate->populate();
   this->ui.skyRegion->populate();
   //
   this->ui.encounterZone->addFormType(dovah::form_type::encounter_zone);
   this->ui.ownerNPC->addFormType(dovah::form_type::actor_base);
   this->ui.ownerFaction->addFormType(dovah::form_type::faction);
   this->ui.interiorLockList->addFormType(dovah::form_type::actor_base);
   this->ui.interiorLockList->addFormType(dovah::form_type::formlist);
   this->ui.encounterZone->setAllowNone(true);
   this->ui.ownerNPC->setAllowNone(true);
   this->ui.ownerFaction->setAllowNone(true);
   this->ui.interiorLockList->setAllowNone(true);
   this->ui.encounterZone->populate();
   this->ui.ownerNPC->populate();
   this->ui.ownerFaction->populate();
   this->ui.interiorLockList->populate();
   QObject::connect(this->ui.ownerFaction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      auto& editor = DovahKitCore::get();
      this->working_ownership.form = editor.get_form(this->ui.ownerFaction->formID());
      this->_update_ownership_widgets();
   });
   QObject::connect(this->ui.ownerNPC, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      auto& editor = DovahKitCore::get();
      this->working_ownership.form = editor.get_form(this->ui.ownerNPC->formID());
      this->_update_ownership_widgets();
   });
   //
   this->load();
}
void FormDialogCell::_update_ownership_widgets() {
   const auto blocker0 = QSignalBlocker(this->ui.ownerFaction);
   const auto blocker1 = QSignalBlocker(this->ui.ownerFactionRequiredRank);
   const auto blocker2 = QSignalBlocker(this->ui.ownerNPC);
   //
   this->ui.ownerFactionRequiredRank->clear();
   //
   this->ui.ownerFaction->setEnabled(true);
   this->ui.ownerNPC->setEnabled(true);
   this->ui.ownerFactionRequiredRank->setEnabled(true);
   this->ui.ownerFaction->setFormByID(0);
   this->ui.ownerNPC->setFormByID(0);
   if (auto stub = this->working_ownership.form) {
      if (stub->formType == dovah::form_type::actor_base) {
         this->ui.ownerNPC->setFormByID(stub->formID);
      } else if (stub->formType == dovah::form_type::faction) {
         this->ui.ownerFaction->setFormByID(stub->formID);
      }
      //
      // TODO: faction rank
      //
   }
   if (this->ui.ownerFaction->formID()) {
      this->ui.ownerNPC->setEnabled(false);
   } else {
      this->ui.ownerFactionRequiredRank->setEnabled(false);
      if (this->ui.ownerNPC->formID())
         this->ui.ownerFaction->setEnabled(false);
   }
}
void FormDialogCell::_load_impl() {
   auto& extra    = this->form->extra_data;
   auto& lighting = this->form->interior.lighting;
   auto& editor   = DovahKitCore::get();
   //
   bool is_exterior = this->stub->is_exterior_cell();
   this->ui.tabs->setTabEnabled(1, !is_exterior);
   this->ui.tabs->setTabEnabled(2, !is_exterior);
   this->ui.tabs->setTabEnabled(3, !is_exterior);
   //
   #pragma region General
      this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
      _load_extra_formID<extra::location, extra_data_type::location>(this->ui.location, extra);
      this->ui.flagCantTravel->setChecked(this->form->cell_flags & cell_flag::cant_travel_from_here);
      this->ui.flagHandChanged->setChecked(this->form->cell_flags & cell_flag::hand_changed);
      _load_extra_formID<extra::cell_acoustic_space, extra_data_type::cell_acoustic_space>(this->ui.acousticSpace, extra);
      _load_extra_formID<extra::cell_imagespace,     extra_data_type::cell_imagespace>    (this->ui.imagespace, extra);
      if (auto* data = extra.lookup<extra::cell_music_override>(extra_data_type::cell_music_override)) {
         this->ui.musicType->setFormByID(data->formID);
      } else {
         this->ui.musicType->setToUndefined();
      }
      if (!this->stub->is_exterior_cell()) {
         this->ui.waterEnabled->setChecked(this->form->cell_flags & cell_flag::has_water);
         this->ui.waterEnabled->setEnabled(true);
      } else {
         this->ui.waterEnabled->setChecked(true);
         this->ui.waterEnabled->setEnabled(false);
      }
      _load_extra_formID<extra::cell_water_type, extra_data_type::cell_water_type>(this->ui.waterType, extra); // only serialized if Has Water is enabled
      this->ui.waterHeight->setValue(this->form->water.height); // only serialized if Has Water is enabled
      //
      this->ui.waterLinearVelocityX->setValue(0.0); // only serialized if Has Water is enabled
      this->ui.waterLinearVelocityY->setValue(0.0);
      this->ui.waterLinearVelocityZ->setValue(0.0);
      this->ui.waterAngularVelocityX->setValue(0.0); // only serialized if Has Water is enabled
      this->ui.waterAngularVelocityY->setValue(0.0);
      this->ui.waterAngularVelocityZ->setValue(0.0);
      if (auto* data = extra.lookup<extra::water_data>(extra_data_type::water_data)) {
         if (data->data.size() > 0) {
            auto& vector4 = data->data[0];
            this->ui.waterLinearVelocityX->setValue(vector4.velocity.x);
            this->ui.waterLinearVelocityY->setValue(vector4.velocity.y);
            this->ui.waterLinearVelocityZ->setValue(vector4.velocity.z);
         }
         if (data->data.size() > 1) {
            auto& vector4 = data->data[1];
            this->ui.waterLinearVelocityX->setValue(vector4.velocity.x);
            this->ui.waterAngularVelocityY->setValue(vector4.velocity.y);
            this->ui.waterAngularVelocityZ->setValue(vector4.velocity.z);
         }
      }
      //
      this->ui.hideLandQuad1->setChecked(this->form->land_flags & land_flag::force_hide_quad_1);
      this->ui.hideLandQuad2->setChecked(this->form->land_flags & land_flag::force_hide_quad_2);
      this->ui.hideLandQuad3->setChecked(this->form->land_flags & land_flag::force_hide_quad_3);
      this->ui.hideLandQuad4->setChecked(this->form->land_flags & land_flag::force_hide_quad_4);
   #pragma endregion
   if (!is_exterior) {
      #pragma region Lighting
         this->ui.lightingTemplate->setFormByID(this->form->interior.lighting_template_ID);
         //
         this->ui.inheritAmbient->setChecked(lighting.inherit_flags & inherit_flag::ambient);
         this->ui.inheritDirectionalColor->setChecked(lighting.inherit_flags & inherit_flag::directional);
         this->ui.inheritDirectionalFade->setChecked(lighting.inherit_flags & inherit_flag::directional_fade);
         this->ui.inheritDirectionalRot->setChecked(lighting.inherit_flags & inherit_flag::directional_rotation);
         this->ui.inheritLightFadeDistances->setChecked(lighting.inherit_flags & inherit_flag::light_fade_distances);
         this->ui.inheritFogColor->setChecked(lighting.inherit_flags & inherit_flag::fog_color);
         this->ui.inheritFogDistanceNear->setChecked(lighting.inherit_flags & inherit_flag::fog_distance_near);
         this->ui.inheritFogDistanceFar->setChecked(lighting.inherit_flags & inherit_flag::fog_distance_far);
         this->ui.inheritFogPower->setChecked(lighting.inherit_flags & inherit_flag::fog_power);
         this->ui.inheritFogMax->setChecked(lighting.inherit_flags & inherit_flag::fog_max);
         //
         this->ui.lightingColorAmbient->setColor(_form_color_to_q(lighting.ambient));
         this->ui.lightingColorDirectional->setColor(_form_color_to_q(lighting.directional));
         this->ui.lightingFogColorNear->setColor(_form_color_to_q(lighting.fog_color_near));
         this->ui.lightingFogColorFar->setColor(_form_color_to_q(lighting.fog_color_far));
         this->ui.lightingFogDistanceNear->setValue(lighting.fog_distance_near);
         this->ui.lightingFogDistanceFar->setValue(lighting.fog_distance_far);
         this->ui.lightingFogPower->setValue(lighting.fog_power);
         this->ui.lightingFogMax->setValue(lighting.fog_max);
         this->ui.lightingFogClipDistance->setValue(lighting.fog_distance_clip);
         //
         this->ui.lightingDirectionalFade->setValue(lighting.directional_fade);
         this->ui.lightingDirectionalRotationXY->setValue(lighting.rotation.xy);
         this->ui.lightingDirectionalRotationZ->setValue(lighting.rotation.z);
         //
         this->ui.skyVisible->setChecked(this->form->cell_flags & cell_flag::show_sky);
         this->ui.skyLighting->setChecked(this->form->cell_flags & cell_flag::use_sky_lighting);
         _load_extra_formID<extra::cell_climate, extra_data_type::cell_climate>(this->ui.skyRegion, extra);
         //
         this->ui.lightingFadeDistanceStart->setValue(lighting.light_fade_distance.start);
         this->ui.lightingFadeDistanceEnd->setValue(lighting.light_fade_distance.end);
      #pragma endregion
      #pragma region Directional Ambient Lighting
         this->ui.directionalAmbColorXPos->setColor(_form_color_to_q(lighting.directional_ambient_colors.x_pos));
         this->ui.directionalAmbColorYPos->setColor(_form_color_to_q(lighting.directional_ambient_colors.y_pos));
         this->ui.directionalAmbColorZPos->setColor(_form_color_to_q(lighting.directional_ambient_colors.z_pos));
         this->ui.directionalAmbColorXNeg->setColor(_form_color_to_q(lighting.directional_ambient_colors.x_neg));
         this->ui.directionalAmbColorYNeg->setColor(_form_color_to_q(lighting.directional_ambient_colors.y_neg));
         this->ui.directionalAmbColorZNeg->setColor(_form_color_to_q(lighting.directional_ambient_colors.z_neg));
      #pragma endregion
      #pragma region Interior Data
         this->ui.name->setText(this->form->name.c_str());
         _load_extra_formID<extra::encounter_zone, extra_data_type::encounter_zone>(this->ui.encounterZone, extra);
         //
         // TODO: water environment map texture path
         //
         //
         this->ui.ownerFactionRequiredRank->clear();
         if (auto* data = extra.lookup<extra::ownership>(extra_data_type::ownership)) {
            this->working_ownership.form = editor.get_form(data->formID);
         }
         if (auto* data = extra.lookup<extra::rank>(extra_data_type::rank)) {
            this->working_ownership.rank = data->value;
         }
         this->_update_ownership_widgets();
         _load_extra_formID<extra::interior_lock_list, extra_data_type::interior_lock_list>(this->ui.interiorLockList, extra);
         this->ui.flagPublicArea->setChecked(this->form->cell_flags & cell_flag::public_area);
         this->ui.flagOffLimits->setChecked(this->form->cell_flags & form_flag::off_limits);
         this->ui.flagCantWait->setChecked(this->form->cell_flags & form_flag::cant_wait);
      #pragma endregion
   }
}
void FormDialogCell::_save_impl() {
   this->stub->editorID = this->ui.editorID->text().toStdString();
   #if !_DEBUG
      static_assert(false, "FINISH ME");
   #endif
   //
   // TODO: 
   // if (this->ui.musicType->isUndefined()) then remove the music-type extra data entirely.
   // otherwise, set the extra-data to whatever form ID was specified, even if it's NONE.
   //
   if (!this->stub->is_exterior_cell()) {
   }
}