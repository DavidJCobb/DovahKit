#include "./cell.h"
#include "helpers/qt/spinbox.h"
#include "helpers/qt/vector3.h"
#include "helpers/bitwise.h"
#include "helpers/miscellaneous.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/extra_data/types/c/cell_acoustic_space.h"
#include "dovah/forms/components/extra_data/types/c/cell_climate.h"
#include "dovah/forms/components/extra_data/types/c/cell_imagespace.h"
#include "dovah/forms/components/extra_data/types/c/cell_music_override.h"
#include "dovah/forms/components/extra_data/types/c/cell_water_type.h"
#include "dovah/forms/components/extra_data/types/e/encounter_zone.h"
#include "dovah/forms/components/extra_data/types/i/interior_lock_list.h"
#include "dovah/forms/components/extra_data/types/l/location.h"
#include "dovah/forms/components/extra_data/types/o/ownership.h"
#include "dovah/forms/components/extra_data/types/r/rank.h"
#include "dovah/forms/components/extra_data/types/w/water_data.h"
#include "dovah/forms/components/extra_data/types/w/water_environment_map.h"

namespace {
   using form_flag = dovah::loaded_forms::Cell::form_flag;
   using cell_flag = dovah::loaded_forms::Cell::cell_flag;
   using land_flag = dovah::loaded_forms::Cell::land_flag;
   using inherit_flag = dovah::loaded_forms::structs::cell_lighting::inherit_flag;

   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }

   template<typename T, class widget_t> void _load_extra_formID(widget_t* widget, dovah::loaded_forms::components::extra_data_list& extra) {
      if (auto* data = extra.get<T>()) {
         widget->setFormStub(data->form.get_form_stub());
      } else {
         widget->setFormStub(nullptr);
      }
   }

   QColor _form_color_to_q(const dovah::loaded_forms::color_t c) {
      return QColor(c.r, c.g, c.b, 255);
   }
   void _q_color_to_form(dovah::loaded_forms::color_t& target, const QColor& c) {
      target.r = c.red();
      target.g = c.green();
      target.b = c.blue();
   }
}
FormDialogCell::FormDialogCell(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   initialize(stub);
   
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->working_ownership.form = nullptr;
      this->working_ownership.loaded_faction = nullptr;
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub == this->working_ownership.form) {
         if (stub->form_type == dovah::form_type::faction)
            this->_update_rank_picker();
      }
   });
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
   this->ui.location->setAllowedFormType(dovah::form_type::location);
   this->ui.acousticSpace->setAllowedFormType(dovah::form_type::acoustic_space);
   this->ui.imagespace->setAllowedFormType(dovah::form_type::imagespace);
   this->ui.musicType->setAllowedFormType(dovah::form_type::music_type);
   this->ui.waterType->setAllowedFormType(dovah::form_type::water_type);
   this->ui.location->setAllowNone(true);
   this->ui.acousticSpace->setAllowNone(true);
   this->ui.imagespace->setAllowNone(true);
   //this->ui.imagespace->setNoneLabel(tr("DEFAULT", "cell imagespace"));
   this->ui.musicType->setAllowNone(true);
   //this->ui.musicType->setNoneLabel(tr("DEFAULT", "cell music"));
   this->ui.waterType->setDefaultForm(editor.get_form(dovah::hardcoded_form_ids::DefaultWater));
   //
   this->ui.lightingTemplate->setAllowedFormType(dovah::form_type::lighting_template);
   this->ui.lightingTemplate->setAllowNone(true);
   this->ui.skyRegion->setAllowedFormType(dovah::form_type::region);
   this->ui.skyRegion->setAllowNone(true);
   //
   this->ui.encounterZone->setAllowedFormType(dovah::form_type::encounter_zone);
   this->ui.ownerNPC->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.ownerFaction->setAllowedFormType(dovah::form_type::faction);
   this->ui.interiorLockList->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.interiorLockList->setAllowedFormType(dovah::form_type::formlist);
   this->ui.encounterZone->setAllowNone(true);
   this->ui.ownerNPC->setAllowNone(true);
   this->ui.ownerFaction->setAllowNone(true);
   this->ui.interiorLockList->setAllowNone(true);
   QObject::connect(this->ui.ownerFaction, &DKFormPicker::formChanged, this, [this]() {
      auto& editor = DovahKitCore::get();
      this->working_ownership.form = this->ui.ownerFaction->formStub();
      this->_update_ownership_widgets();
   });
   QObject::connect(this->ui.ownerNPC, &DKFormPicker::formChanged, this, [this]() {
      auto& editor = DovahKitCore::get();
      this->working_ownership.form = this->ui.ownerNPC->formStub();
      this->_update_ownership_widgets();
   });
   //
   this->load();
   this->_update_ownership_widgets();
   this->_update_rank_picker();
}
void FormDialogCell::_update_ownership_widgets() {
   const auto blocker0 = QSignalBlocker(this->ui.ownerFaction);
   const auto blocker1 = QSignalBlocker(this->ui.ownerFactionRequiredRank);
   const auto blocker2 = QSignalBlocker(this->ui.ownerNPC);
   //
   dovah::form_stub* faction_stub = nullptr;
   //
   this->ui.ownerFactionRequiredRank->clear();
   //
   this->ui.ownerFaction->setEnabled(true);
   this->ui.ownerNPC->setEnabled(true);
   this->ui.ownerFactionRequiredRank->setEnabled(true);
   this->ui.ownerFaction->setFormStub(this->working_ownership.form); // the control will filter for us
   this->ui.ownerNPC->setFormStub(this->working_ownership.form); // the control will filter for us
   if (auto* stub = this->working_ownership.form) {
      if (stub->form_type == dovah::form_type::faction)
         faction_stub = stub;
   }
   //
   {
      dovah::form_stub* prior_stub = nullptr;
      if (this->working_ownership.loaded_faction)
         prior_stub = &this->working_ownership.loaded_faction->stub;
      //
      if (prior_stub != faction_stub) {
         if (faction_stub)
            this->working_ownership.loaded_faction = faction_stub->load().ptr_cast<dovah::loaded_forms::Faction>();
         else
            this->working_ownership.loaded_faction = nullptr;
         this->_update_rank_picker();
      }
   }
   //
   if (this->ui.ownerFaction->formStub()) {
      this->ui.ownerNPC->setEnabled(false);
   } else {
      this->ui.ownerFactionRequiredRank->setEnabled(false);
      if (this->ui.ownerNPC->formStub())
         this->ui.ownerFaction->setEnabled(false);
   }
}
void FormDialogCell::_update_rank_picker() {
   auto* widget = this->ui.ownerFactionRequiredRank;
   const auto blocker = QSignalBlocker(widget);
   if (this->working_ownership.loaded_faction) {
      widget->setEnabled(true);
      widget->clear();
      //
      auto& list = this->working_ownership.loaded_faction->ranks;
      for (auto& rank : list) {
         QString fem  = rank.title_fem.c_str();
         QString masc = rank.title_masc.c_str();
         //
         QString text = fem;
         if (masc != fem) {
            text = tr((const char*)u8"%1 (\x2640) / %2 (\x2642)", "ownership required rank").arg(fem).arg(masc);
         }
         widget->addItem(text, rank.id);
      }
   } else {
      widget->setEnabled(false);
      widget->clear();
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
      this->ui.editorID->setText(QString::fromStdString(this->form->stub.get_editor_id()));
      _load_extra_formID<extra_data_types::location>(this->ui.location, extra);
      this->ui.flagCantTravel->setChecked(this->form->cell_flags & cell_flag::cant_travel_from_here);
      this->ui.flagHandChanged->setChecked(this->form->cell_flags & cell_flag::hand_changed);
      _load_extra_formID<extra_data_types::cell_acoustic_space>(this->ui.acousticSpace, extra);
      _load_extra_formID<extra_data_types::cell_imagespace>    (this->ui.imagespace, extra);
      if (auto* data = extra.get<extra_data_types::cell_music_override>()) {
         this->ui.musicType->setFormStub(data->form.get_form_stub());
      } else {
         this->ui.musicType->setFormStub(nullptr);
      }
      if (!is_exterior) {
         this->ui.waterEnabled->setChecked(this->form->cell_flags & cell_flag::has_water);
         this->ui.waterEnabled->setEnabled(true);
      } else {
         this->ui.waterEnabled->setChecked(true);
         this->ui.waterEnabled->setEnabled(false);
      }
      this->ui.flagNoLODWater->setChecked(this->form->cell_flags& cell_flag::no_lod_water);
      _load_extra_formID<extra_data_types::cell_water_type>(this->ui.waterType, extra); // only serialized if Has Water is enabled
      this->ui.waterHeight->setValue(this->form->water.height); // only serialized if Has Water is enabled
      //
      this->ui.waterLinearVelocityX->setValue(0.0); // only serialized if Has Water is enabled
      this->ui.waterLinearVelocityY->setValue(0.0);
      this->ui.waterLinearVelocityZ->setValue(0.0);
      this->ui.waterAngularVelocityX->setValue(0.0); // only serialized if Has Water is enabled
      this->ui.waterAngularVelocityY->setValue(0.0);
      this->ui.waterAngularVelocityZ->setValue(0.0);
      if (auto* data = extra.get<extra_data_types::water_data>()) {
         auto size = data->data.size();
         if (size > 0) {
            auto& vector4 = data->data[0];
            cobb::qt::bring_vector3_to_ui(vector4.velocity, this->ui.waterLinearVelocityX, this->ui.waterLinearVelocityY, this->ui.waterLinearVelocityZ);
         }
         if (size > 1) {
            auto& vector4 = data->data[1];
            cobb::qt::bring_vector3_to_ui(vector4.velocity, this->ui.waterAngularVelocityX, this->ui.waterAngularVelocityY, this->ui.waterAngularVelocityZ);
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
         this->ui.lightingTemplate->setFormStub(this->form->interior.lighting_template.get_form_stub());
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
         this->ui.lightingColorAmbient->setColor(_form_color_to_q(lighting.ambient.base));
         this->ui.lightingColorDirectional->setColor(_form_color_to_q(lighting.directional.color));
         this->ui.lightingFogColorNear->setColor(_form_color_to_q(lighting.fog.colors.near));
         this->ui.lightingFogColorFar->setColor(_form_color_to_q(lighting.fog.colors.far));
         this->ui.lightingFogDistanceNear->setValue(lighting.fog.near);
         this->ui.lightingFogDistanceFar->setValue(lighting.fog.far);
         this->ui.lightingFogPower->setValue(lighting.fog.power);
         this->ui.lightingFogMax->setValue(lighting.fog.max);
         this->ui.lightingFogClipDistance->setValue(lighting.fog.clip_distance);
         //
         this->ui.lightingDirectionalFade->setValue(lighting.directional.fade);
         this->ui.lightingDirectionalRotationXY->setValue(lighting.directional.rotation.xy);
         this->ui.lightingDirectionalRotationZ->setValue(lighting.directional.rotation.z);
         //
         this->ui.skyVisible->setChecked(this->form->cell_flags & cell_flag::show_sky);
         this->ui.skyLighting->setChecked(this->form->cell_flags & cell_flag::use_sky_lighting);
         _load_extra_formID<extra_data_types::cell_climate>(this->ui.skyRegion, extra);
         //
         this->ui.lightingFadeDistanceStart->setValue(lighting.light_fade_distance.start);
         this->ui.lightingFadeDistanceEnd->setValue(lighting.light_fade_distance.end);
      #pragma endregion
      #pragma region Directional Ambient Lighting
         this->ui.directionalAmbColorXPos->setColor(_form_color_to_q(lighting.ambient.directional.x.positive));
         this->ui.directionalAmbColorYPos->setColor(_form_color_to_q(lighting.ambient.directional.y.positive));
         this->ui.directionalAmbColorZPos->setColor(_form_color_to_q(lighting.ambient.directional.z.positive));
         this->ui.directionalAmbColorXNeg->setColor(_form_color_to_q(lighting.ambient.directional.x.negative));
         this->ui.directionalAmbColorYNeg->setColor(_form_color_to_q(lighting.ambient.directional.y.negative));
         this->ui.directionalAmbColorZNeg->setColor(_form_color_to_q(lighting.ambient.directional.z.negative));
      #pragma endregion
      #pragma region Interior Data
         this->ui.name->setText(editor.convert_localized_string(this->form->name));
         _load_extra_formID<extra_data_types::encounter_zone>(this->ui.encounterZone, extra);
         if (auto* data = extra.get<extra_data_types::water_environment_map>()) {
            this->ui.waterEnvironmentMap->setValue(data->value.c_str());
         }
         this->ui.ownerFactionRequiredRank->clear();
         if (auto* data = extra.get<extra_data_types::ownership>()) {
            this->working_ownership.form = data->form.get_form_stub();
         }
         if (auto* data = extra.get<extra_data_types::rank>()) {
            this->working_ownership.rank = data->value;
         }
         this->_update_ownership_widgets();
         _load_extra_formID<extra_data_types::interior_lock_list>(this->ui.interiorLockList, extra);
         this->ui.flagPublicArea->setChecked(this->form->cell_flags & cell_flag::public_area);
         this->ui.flagOffLimits->setChecked(this->stub->test_record_flags(form_flag::off_limits));
         this->ui.flagCantWait->setChecked(this->stub->test_record_flags(form_flag::cant_wait));
      #pragma endregion
   }
}
void FormDialogCell::_save_impl() {
   auto& working  = *this->form;
   auto& extra    = this->form->extra_data;
   auto& lighting = this->form->interior.lighting;
   auto& editor   = DovahKitCore::get();
   //
   bool is_exterior = this->stub->is_exterior_cell();
   //
   #pragma region General
      this->stub->editorID = this->ui.editorID->text().toStdString();
      //
      this->write_extra_form_ref<extra_data_types::location>(extra, this->ui.location->formStub());
      cobb::edit_bit(this->form->cell_flags, cell_flag::cant_travel_from_here, this->ui.flagCantTravel->isChecked());
      cobb::edit_bit(this->form->cell_flags, cell_flag::hand_changed,          this->ui.flagHandChanged->isChecked());
      this->write_extra_form_ref<extra_data_types::cell_acoustic_space>(extra, this->ui.acousticSpace->formStub());
      this->write_extra_form_ref<extra_data_types::cell_imagespace>(    extra, this->ui.imagespace->formStub());
      this->write_extra_form_ref<extra_data_types::cell_music_override>(extra, this->ui.musicType->formStub());
      //
      bool has_water = is_exterior || this->ui.waterEnabled->isChecked();
      cobb::edit_bit(this->form->cell_flags, cell_flag::has_water,    has_water);
      cobb::edit_bit(this->form->cell_flags, cell_flag::no_lod_water, this->ui.flagNoLODWater->isChecked());
      if (has_water) {
         this->write_extra_form_ref<extra_data_types::cell_water_type>(extra, this->ui.waterType->formStub());
         this->form->water.height = this->ui.waterHeight->value();
         //
         auto* data = extra.get_or_create<extra_data_types::water_data>();
         if (data) {
            data->data.resize(3);
            //
            auto& linear = data->data[0];
            cobb::qt::get_vector3_from_ui(linear.velocity, this->ui.waterLinearVelocityX, this->ui.waterLinearVelocityY, this->ui.waterLinearVelocityZ);
            linear.unk0C = 0.0F;
            //
            auto& angular = data->data[1];
            cobb::qt::get_vector3_from_ui(angular.velocity, this->ui.waterAngularVelocityX, this->ui.waterAngularVelocityY, this->ui.waterAngularVelocityZ);
            angular.unk0C = 0.0F;
            //
            auto& unknown = data->data[2];
            unknown.velocity.x = 0.0F;
            unknown.velocity.y = 0.0F;
            unknown.velocity.z = 0.0F;
            unknown.unk0C      = 0.0F;
         }
      } else {
         extra.remove<extra_data_types::cell_water_type>(working);
         extra.remove<extra_data_types::water_data>(working);
      }
      //
      cobb::edit_bit(this->form->land_flags, land_flag::force_hide_quad_1, this->ui.hideLandQuad1->isChecked());
      cobb::edit_bit(this->form->land_flags, land_flag::force_hide_quad_2, this->ui.hideLandQuad2->isChecked());
      cobb::edit_bit(this->form->land_flags, land_flag::force_hide_quad_3, this->ui.hideLandQuad3->isChecked());
      cobb::edit_bit(this->form->land_flags, land_flag::force_hide_quad_4, this->ui.hideLandQuad4->isChecked());
   #pragma endregion
   if (!is_exterior) {
      #pragma region Lighting
         this->write_form_ref(this->form->interior.lighting_template, this->ui.lightingTemplate->formStub());
         //
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::ambient,              this->ui.inheritAmbient->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::directional,          this->ui.inheritDirectionalColor->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::directional_fade,     this->ui.inheritDirectionalFade->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::directional_rotation, this->ui.inheritDirectionalRot->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::light_fade_distances, this->ui.inheritLightFadeDistances->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::fog_color,            this->ui.inheritFogColor->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::fog_distance_near,    this->ui.inheritFogDistanceNear->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::fog_distance_far,     this->ui.inheritFogDistanceFar->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::fog_power,            this->ui.inheritFogPower->isChecked());
         cobb::edit_bit(lighting.inherit_flags, inherit_flag::fog_max,              this->ui.inheritFogMax->isChecked());
         //
         _q_color_to_form(lighting.ambient.base,      this->ui.lightingColorAmbient->color());
         _q_color_to_form(lighting.directional.color, this->ui.lightingColorDirectional->color());
         _q_color_to_form(lighting.fog.colors.near,   this->ui.lightingFogColorNear->color());
         _q_color_to_form(lighting.fog.colors.far,    this->ui.lightingFogColorFar->color());
         lighting.fog.near          = this->ui.lightingFogDistanceNear->value();
         lighting.fog.far           = this->ui.lightingFogDistanceFar->value();
         lighting.fog.power         = this->ui.lightingFogPower->value();
         lighting.fog.max           = this->ui.lightingFogMax->value();
         lighting.fog.clip_distance = this->ui.lightingFogClipDistance->value();
         //
         lighting.directional.fade        = this->ui.lightingDirectionalFade->value();
         lighting.directional.rotation.xy = this->ui.lightingDirectionalRotationXY->value();
         lighting.directional.rotation.z  = this->ui.lightingDirectionalRotationZ->value();
         //
         cobb::edit_bit(this->form->cell_flags, cell_flag::show_sky,         this->ui.skyVisible->isChecked());
         cobb::edit_bit(this->form->cell_flags, cell_flag::use_sky_lighting, this->ui.skyLighting->isChecked());
         this->write_extra_form_ref<extra_data_types::cell_climate>(extra, this->ui.skyRegion->formStub());
         //
         lighting.light_fade_distance.start = this->ui.lightingFadeDistanceStart->value();
         lighting.light_fade_distance.end   = this->ui.lightingFadeDistanceEnd->value();
      #pragma endregion
      #pragma region Directional Ambient Lighting
         _q_color_to_form(lighting.ambient.directional.x.positive, this->ui.directionalAmbColorXPos->color());
         _q_color_to_form(lighting.ambient.directional.y.positive, this->ui.directionalAmbColorYPos->color());
         _q_color_to_form(lighting.ambient.directional.z.positive, this->ui.directionalAmbColorZPos->color());
         _q_color_to_form(lighting.ambient.directional.x.negative, this->ui.directionalAmbColorXNeg->color());
         _q_color_to_form(lighting.ambient.directional.y.negative, this->ui.directionalAmbColorYNeg->color());
         _q_color_to_form(lighting.ambient.directional.z.negative, this->ui.directionalAmbColorZNeg->color());
      #pragma endregion
      #pragma region Interior Data
         editor.assign_localized_string(this->form->name, this->ui.name->text());
         this->write_extra_form_ref<extra_data_types::encounter_zone>(extra, this->ui.encounterZone->formStub());
         {
            auto path = this->ui.waterEnvironmentMap->value().to_string();
            if (path.isEmpty()) {
               extra.remove<extra_data_types::water_environment_map>(working);
            } else {
               auto* data = extra.get_or_create<extra_data_types::water_environment_map>();
               data->value = path.toStdString();
            }
         }
         extra.remove<extra_data_types::ownership>(working);
         extra.remove<extra_data_types::rank>(working);
         if (auto* stub = this->working_ownership.form) {
            auto* data = extra.get_or_create<extra_data_types::ownership>();
            if (data) {
               this->write_form_ref(data->form, stub);
               if (stub->form_type == dovah::form_type::faction) {
                  auto* data = extra.get_or_create<extra_data_types::rank>();
                  data->value = this->working_ownership.rank;
               }
            }
         }
         this->write_extra_form_ref<extra_data_types::interior_lock_list>(extra, this->ui.interiorLockList->formStub());
         cobb::edit_bit(this->form->cell_flags, cell_flag::public_area, this->ui.flagPublicArea->isChecked());
         this->stub->edit_record_flags(form_flag::off_limits, this->ui.flagOffLimits->isChecked());
         this->stub->edit_record_flags(form_flag::cant_wait,  this->ui.flagCantWait->isChecked());
      #pragma endregion
   } else {
      //
      // Strip interior-specific data off of this exterior cell.
      //
      #pragma region Lighting
         this->write_form_ref(this->form->interior.lighting_template, nullptr);
         cobb::edit_bit(this->form->cell_flags, cell_flag::show_sky,         false);
         cobb::edit_bit(this->form->cell_flags, cell_flag::use_sky_lighting, false);
         extra.remove<extra_data_types::cell_climate>(working);
      #pragma endregion
      #pragma region Interior Data
         extra.remove<extra_data_types::encounter_zone>(working);
         extra.remove<extra_data_types::ownership>(working);
         extra.remove<extra_data_types::rank>(working);
         extra.remove<extra_data_types::interior_lock_list>(working);
         cobb::edit_bit(this->form->cell_flags, cell_flag::public_area, false);
         this->stub->edit_record_flags(form_flag::off_limits, false);
         this->stub->edit_record_flags(form_flag::cant_wait,  false);
      #pragma endregion
   }
}