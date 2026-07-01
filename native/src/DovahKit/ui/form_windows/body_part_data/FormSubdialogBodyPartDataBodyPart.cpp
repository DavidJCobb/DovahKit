#include "./FormSubdialogBodyPartDataBodyPart.h"
#include "dovah/data/actor_values.h"
#include "dovah/data/limbs.h"
#include "editor/core.h"
#include "editor/localize/limb.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/set_range.h"
#include "./SkeletonBonesModel.h"

FormSubdialogBodyPartDataBodyPart::FormSubdialogBodyPartDataBodyPart(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   this->setWindowFlags(this->windowFlags() | Qt::WindowContextHelpButtonHint); // show "What's This?" button in title bar
   QObject::connect(this->ui.buttonOK,     &QPushButton::pressed, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::pressed, this, &QDialog::reject);

   {  // Limbs
      auto* widget = this->ui.limb;
      widget->clear();
      for (size_t i = 0; i < dovah::limbs_count; ++i) {
         widget->addItem(editor::localize::limb((dovah::limb)i), i);
      }
      widget->model()->sort(0);
   }

   this->ui.actorValue->setAllowedFormType(dovah::form_type::actor_value_info);
   this->ui.explodeDebris->setAllowedFormType(dovah::form_type::debris);
   this->ui.explodeForm->setAllowedFormType(dovah::form_type::explosion);
   this->ui.explodeImpactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
   this->ui.severDebris->setAllowedFormType(dovah::form_type::debris);
   this->ui.severExplosion->setAllowedFormType(dovah::form_type::explosion);
   this->ui.severImpactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);

   ui::set_unsigned_range<float>(this->ui.damageMult);
   this->ui.healthPerc->setRange(0, 100);
   this->ui.chanceToHit->setRange(0, 100);
   ui::set_range<float>(this->ui.headtrackMaxAngle);
   ui::set_range<uint8_t>(this->ui.severDebrisCount);
   ui::set_unsigned_range<float>(this->ui.severDebrisScale);
   ui::set_range<uint8_t>(this->ui.severDecalCount);
   this->ui.explodeChance->setRange(0, 100);
   ui::set_range<uint8_t>(this->ui.explodeDebrisCount);
   ui::set_unsigned_range<float>(this->ui.explodeDebrisScale);
   ui::set_unsigned_range<float>(this->ui.explodeReplaceScale);
   ui::set_range<uint8_t>(this->ui.explodeDecalCount);
   for (auto* widget : std::array{
      this->ui.goreEffectsPosX,
      this->ui.goreEffectsPosY,
      this->ui.goreEffectsPosZ,
      this->ui.goreEffectsRotX,
      this->ui.goreEffectsRotY,
      this->ui.goreEffectsRotZ,
   }) {
      ui::set_range<float>(widget);
   }
}

SkeletonBonesModel* FormSubdialogBodyPartDataBodyPart::bonesModel() const {
   return this->bones_model;
}
void FormSubdialogBodyPartDataBodyPart::setBonesModel(SkeletonBonesModel* v) {
   if (v == this->bones_model)
      return;
   this->bones_model = v;
   for (auto* widget : std::array{
      this->ui.partNode,
      this->ui.vatsNode,
      this->ui.headtrackStartNode,
      this->ui.goreEffectsBone,
   }) {
      widget->setModel(v);
   }
}

FormSubdialogBodyPartDataBodyPart::loaded_item_type FormSubdialogBodyPartDataBodyPart::value() const {
   loaded_item_type dst;
   dst.flags = 0;

   dovahkit::subsystems::game_localized_strings::core::get().assign_localized_string(dst.name, this->ui.name->text());
   dst.limb = (dovah::limb)this->ui.limb->currentData().toInt();
   dst.nodes.main       = this->ui.partNode->currentText().toStdString();
   dst.nodes.vats_target = this->ui.vatsNode->currentText().toStdString();
   #pragma region Combat data
      {  // Actor Value
         auto* avi = this->ui.actorValue->formStub();
         if (avi) {
            for (auto& dfn : dovah::all_actor_value_info) {
               if (dfn.formID == avi->formID) {
                  dst.combat.actor_value_id = dfn.index;
                  break;
               }
            }
         } else {
            dst.combat.actor_value_id = -1;
         }
      }
      dst.combat.chance_to_hit  = this->ui.chanceToHit->value();
      dst.combat.damage_mult    = this->ui.damageMult->value();
      dst.combat.health_percent = this->ui.healthPerc->value();
   #pragma endregion
   #pragma region IK Data
      if (this->ui.flagIKData->isChecked())
         dst.flags |= loaded_item_type::flag::has_ik_data;
      if (this->ui.flagBipedData->isChecked())
         dst.flags |= loaded_item_type::flag::ik_data_biped_data;
      if (this->ui.flagIsHead->isChecked())
         dst.flags |= loaded_item_type::flag::ik_data_is_head;
      if (this->ui.flagHeadtracking->isChecked())
         dst.flags |= loaded_item_type::flag::ik_data_headtracking;
      dst.headtracking_max_angle = this->ui.headtrackMaxAngle->value();
      dst.nodes.ik_start = this->ui.headtrackStartNode->currentText().toStdString();
   #pragma endregion
   #pragma region Pose Matching
      dst.pose_matching = this->ui.poseMatching->value().lexically_relative("Data\\Meshes").to_string().toStdString();
   #pragma endregion
   #pragma region Severable
   {
      if (this->ui.flagSeverable->isChecked())
         dst.flags |= loaded_item_type::flag::severable;

      auto& dst_obj = dst.gore.severable;
      dst_obj.debris          = this->ui.severDebris->formStub();
      dst_obj.debris_count    = this->ui.severDebrisCount->value();
      dst_obj.debris_scale    = this->ui.severDebrisScale->value();
      dst_obj.decal_count     = this->ui.severDecalCount->value();
      dst_obj.explosion       = this->ui.severExplosion->formStub();
      dst_obj.impact_data_set = this->ui.severImpactDataSet->formStub();
   }
   #pragma endregion
   #pragma region Explodable
   {
      if (this->ui.flagExplodable->isChecked())
         dst.flags |= loaded_item_type::flag::explodable;

      auto& dst_obj = dst.gore.explodable;
      dst_obj.chance          = this->ui.explodeChance->value();
      dst_obj.debris          = this->ui.explodeDebris->formStub();
      dst_obj.debris_count    = this->ui.explodeDebrisCount->value();
      dst_obj.debris_scale    = this->ui.explodeDebrisScale->value();
      dst_obj.decal_count     = this->ui.explodeDecalCount->value();
      dst_obj.explosion       = this->ui.explodeForm->formStub();
      dst_obj.impact_data_set = this->ui.explodeImpactDataSet->formStub();

      if (this->ui.explodeChanceAbs->isChecked())
         dst.flags |= loaded_item_type::flag::absolute_explode_chance;

      dst_obj.limb_replacement.model.model_path = this->ui.explodeReplaceNIF->value().lexically_relative("Data\\Meshes\\").to_string().toStdString();
      dst_obj.limb_replacement.scale = this->ui.explodeReplaceScale->value();
   }
   #pragma endregion
   #pragma region Gore effects positioning
   {
      auto& dst_obj = dst.gore.effect_positioning;
      dst.nodes.gore_effect = this->ui.goreEffectsBone->currentText().toStdString();
      dst_obj.pos.x = this->ui.goreEffectsPosX->value();
      dst_obj.pos.y = this->ui.goreEffectsPosY->value();
      dst_obj.pos.z = this->ui.goreEffectsPosZ->value();
      dst_obj.rot.x = this->ui.goreEffectsRotX->value();
      dst_obj.rot.y = this->ui.goreEffectsRotY->value();
      dst_obj.rot.z = this->ui.goreEffectsRotZ->value();
   }
   #pragma endregion

   return dst;
}
void FormSubdialogBodyPartDataBodyPart::setValue(const loaded_item_type& src) {
   int index_of_base_node_name = -1;
   if (this->bones_model) {
      index_of_base_node_name = this->bones_model->baseNodeRow();
   }

   auto _set_node_combobox = [index_of_base_node_name](QComboBox& widget, const std::string& node_name) {
      auto i = widget.findText(QString::fromStdString(node_name));
      if (i >= 0)
         widget.setCurrentIndex(i);
      else
         widget.setCurrentIndex(index_of_base_node_name);
   };

   this->ui.name->setText(dovahkit::subsystems::game_localized_strings::core::get().convert_localized_string(src.name));
   {
      auto* widget = this->ui.limb;
      auto  value  = (int)src.limb;
      int   i = widget->findData(value);
      if (i >= 0)
         widget->setCurrentIndex(i);
   }
   _set_node_combobox(*this->ui.partNode, src.nodes.main);
   _set_node_combobox(*this->ui.vatsNode, src.nodes.vats_target);
   #pragma region Combat data
      {
         const auto av_id = src.combat.actor_value_id;
         if (av_id == (uint8_t)-1) {
            this->ui.actorValue->setFormStub(nullptr);
         } else {
            dovah::form_stub* stub = nullptr;
            if (av_id < dovah::all_actor_value_info.size()) {
               auto info_id = dovah::all_actor_value_info[av_id].formID;
               stub = DovahKitCore::get().get_form_of_probable_type(dovah::form_type::actor_value_info, info_id);
            }
            this->ui.actorValue->setFormStub(stub);
         }
      }
      this->ui.chanceToHit->setValue(src.combat.chance_to_hit);
      this->ui.damageMult->setValue(src.combat.damage_mult);
      this->ui.healthPerc->setValue(src.combat.health_percent);
   #pragma endregion
   #pragma region IK Data
      this->ui.flagIKData->setChecked(src.flags & loaded_item_type::flag::has_ik_data);
      this->ui.flagBipedData->setChecked(src.flags & loaded_item_type::flag::ik_data_biped_data);
      this->ui.flagIsHead->setChecked(src.flags & loaded_item_type::flag::ik_data_is_head);
      this->ui.flagHeadtracking->setChecked(src.flags & loaded_item_type::flag::ik_data_headtracking);
      this->ui.headtrackMaxAngle->setValue(src.headtracking_max_angle);
      _set_node_combobox(*this->ui.headtrackStartNode, src.nodes.ik_start);
   #pragma endregion
   #pragma region Pose Matching
      {
         ui::types::game_file_path path("Data\\Meshes");
         path.append(src.pose_matching.c_str());
         this->ui.poseMatching->setValue(path);
      }
   #pragma endregion
   #pragma region Severable
   {
      this->ui.flagSeverable->setChecked(src.flags & loaded_item_type::flag::severable);

      auto& src_obj = src.gore.severable;
      this->ui.severDebris->setFormStub(src_obj.debris);
      this->ui.severDebrisCount->setValue(src_obj.debris_count);
      this->ui.severDebrisScale->setValue(src_obj.debris_scale);
      this->ui.severDecalCount->setValue(src_obj.decal_count);
      this->ui.severExplosion->setFormStub(src_obj.explosion);
      this->ui.severImpactDataSet->setFormStub(src_obj.impact_data_set);
   }
   #pragma endregion
   #pragma region Explodable
   {
      this->ui.flagExplodable->setChecked(src.flags & loaded_item_type::flag::explodable);

      auto& src_obj = src.gore.explodable;
      this->ui.explodeChance->setValue(src_obj.chance);
      this->ui.explodeDebris->setFormStub(src_obj.debris);
      this->ui.explodeDebrisCount->setValue(src_obj.debris_count);
      this->ui.explodeDebrisScale->setValue(src_obj.debris_scale);
      this->ui.explodeDecalCount->setValue(src_obj.decal_count);
      this->ui.explodeForm->setFormStub(src_obj.explosion);
      this->ui.explodeImpactDataSet->setFormStub(src_obj.impact_data_set);

      this->ui.explodeChanceAbs->setChecked(src.flags & loaded_item_type::flag::absolute_explode_chance);

      {
         ui::types::game_file_path path("Data\\Meshes");
         path.append(src_obj.limb_replacement.model.model_path.c_str());
         this->ui.explodeReplaceNIF->setValue(path);
      }
      this->ui.explodeReplaceScale->setValue(src_obj.limb_replacement.scale);
   }
   #pragma endregion
   #pragma region Gore effects positioning
   {
      auto& src_obj = src.gore.effect_positioning;
      _set_node_combobox(*this->ui.goreEffectsBone, src.nodes.gore_effect);
      this->ui.goreEffectsPosX->setValue(src_obj.pos.x);
      this->ui.goreEffectsPosY->setValue(src_obj.pos.y);
      this->ui.goreEffectsPosZ->setValue(src_obj.pos.z);
      this->ui.goreEffectsRotX->setValue(src_obj.rot.x);
      this->ui.goreEffectsRotY->setValue(src_obj.rot.y);
      this->ui.goreEffectsRotZ->setValue(src_obj.rot.z);
   }
   #pragma endregion
}
