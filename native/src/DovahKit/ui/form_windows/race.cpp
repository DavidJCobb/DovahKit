#include "./race.h"
#include <limits>
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/data/skills.h"
#include "dovah/core.h"
#include "editor/helpers/skill_name_to_string.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"

#include "./shared/FaceBaseHeadPartsModel.h"
#include "./shared/FaceExtraHeadPartsModel.h"
#include "./shared/HeadPartPickerFilter.h"

FormDialogRace::FormDialogRace(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   auto& editor = DovahKitCore::get();

   #pragma region General tab
      // Inherit
      this->ui.morphRace->setAllowedFormType(dovah::form_type::race);
      this->ui.armorRace->setAllowedFormType(dovah::form_type::race);

      // Miscellaneous
      {
         auto* widget = this->ui.creatureSize;
         widget->clear();
         widget->addItem(tr("Small"),      (int)loaded_form_type::creature_size::small);
         widget->addItem(tr("Medium"),     (int)loaded_form_type::creature_size::medium);
         widget->addItem(tr("Large"),      (int)loaded_form_type::creature_size::large);
         widget->addItem(tr("Very Large"), (int)loaded_form_type::creature_size::extra_large);
      }
      ui::set_unsigned_range<float>(this->ui.flightRadius);
      ui::set_unsigned_range<float>(this->ui.baseCarryCapacity);
      
      #pragma region Skill boost widgets
         this->_subwidgets.skills.which = {
            this->ui.skillBonus01Skill,
            this->ui.skillBonus02Skill,
            this->ui.skillBonus03Skill,
            this->ui.skillBonus04Skill,
            this->ui.skillBonus05Skill,
            this->ui.skillBonus06Skill,
            this->ui.skillBonus07Skill,
         };
         this->_subwidgets.skills.boost = {
            this->ui.skillBonus01Value,
            this->ui.skillBonus02Value,
            this->ui.skillBonus03Value,
            this->ui.skillBonus04Value,
            this->ui.skillBonus05Value,
            this->ui.skillBonus06Value,
            this->ui.skillBonus07Value,
         };
         for (auto* widget : this->_subwidgets.skills.which) {
            widget->addItem(tr("None"), -1);
            for (size_t i = 0; i < dovah::skill_count; ++i) {
               auto skill = (dovah::skill)i;
               auto name  = editor_helpers::skill_name_to_string(skill);
               widget->addItem(name, (int)skill);
            }
         }
         for (auto* widget : this->_subwidgets.skills.boost) {
            ui::set_range<loaded_form_type::skill_boost_value_type>(widget);
         }
      #pragma endregion

      // Attributes
      ui::set_unsigned_range<float>(this->ui.attrBaseH);
      ui::set_unsigned_range<float>(this->ui.attrBaseM);
      ui::set_unsigned_range<float>(this->ui.attrBaseS);
      ui::set_unsigned_range<float>(this->ui.attrRegenH);
      ui::set_unsigned_range<float>(this->ui.attrRegenM);
      ui::set_unsigned_range<float>(this->ui.attrRegenS);

      // Spells
      this->ui.abilities->setAllowedFormTypes({ dovah::form_type::spell, dovah::form_type::leveled_spell, dovah::form_type::shout });

      ui::set_range<float>(this->ui.mountOffsetX);
      ui::set_range<float>(this->ui.mountOffsetY);
      ui::set_range<float>(this->ui.mountOffsetZ);
      ui::set_range<float>(this->ui.dismountOffsetX);
      ui::set_range<float>(this->ui.dismountOffsetY);
      ui::set_range<float>(this->ui.dismountOffsetZ);
      ui::set_range<float>(this->ui.mountCamOffsetX);
      ui::set_range<float>(this->ui.mountCamOffsetY);
      ui::set_range<float>(this->ui.mountCamOffsetZ);
   #pragma endregion
   #pragma region Body tab
      ui::set_unsigned_range<float>(this->ui.mass);
      this->ui.bodyPartData->setAllowedFormType(dovah::form_type::body_part_data);
      this->ui.skin->setAllowedFormType(dovah::form_type::armor);

      ui::set_unsigned_range<float>(this->ui.heightMultF);
      ui::set_unsigned_range<float>(this->ui.heightMultM);
      this->ui.bodyWeightF->setRange(0, 100);
      this->ui.bodyWeightM->setRange(0, 100);
      static_assert(false, "TODO: Skeleton (limit file extension?)");
      static_assert(false, "TODO: Behavior graph (limit file extension?)");
      static_assert(false, "TODO: Body Texture (limit file extension?)");
      this->ui.decapArmorF->setAllowedFormType(dovah::form_type::armor);
      this->ui.decapArmorM->setAllowedFormType(dovah::form_type::armor);
      this->ui.voicetypeF->setAllowedFormType(dovah::form_type::voicetype);
      this->ui.voicetypeM->setAllowedFormType(dovah::form_type::voicetype);
      this->ui.voicetypeF->setDefaultForm(editor.get_form_of_probable_type(dovah::form_type::voicetype, dovah::hardcoded_form_ids::AdultFemaleVoice1));
      this->ui.voicetypeM->setDefaultForm(editor.get_form_of_probable_type(dovah::form_type::voicetype, dovah::hardcoded_form_ids::AdultMaleVoice1));
      this->ui.voicetypeF->setAllowNone(false);
      this->ui.voicetypeM->setAllowNone(false);

      static_assert(false, "TODO: Body Slot");
      static_assert(false, "TODO: Hair Slot");
      static_assert(false, "TODO: Head Slot");
      static_assert(false, "TODO: Shield Slot");
      static_assert(false, "TODO: Slot Names");
      static_assert(false, "TODO: Visible in First Person");
   #pragma endregion
   #pragma region Blood tab
      this->ui.impactMaterialType->setAllowedFormType(dovah::form_type::material_type);
      this->ui.decapBloodArt->setAllowedFormType(dovah::form_type::art_object);
      this->ui.impactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
      this->ui.soundOpen->setAllowedFormType(dovah::form_type::sound_descriptor);
      this->ui.soundClose->setAllowedFormType(dovah::form_type::sound_descriptor);
   #pragma endregion
   #pragma region Text tab
      //
      // No setup needed at this time.
      //
   #pragma endregion
   #pragma region Movement Details tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Attack Data tab
      //
      // Just the one premade widget. It'll set itself up for us.
      //
   #pragma endregion
   #pragma region Combat tab
      static_assert(false, "TODO");
      this->ui.unarmedEquipSlot->setAllowedFormType(dovah::form_type::equip_slot);
      static_assert(false, "TODO: equipment restrictions");
   #pragma endregion
   #pragma region Lip Synching tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Data tab
      {  // Base Head Parts
         auto _configure = [this](
            dovah::sex    sex,
            QTableView*   view,
            DKFormPicker* picker
         ) {
            auto* model  = this->_models.head_parts_base[sex] = new FaceBaseHeadPartsModel(this);
            view->setModel(model);
            view->setAcceptDrops(true);
            view->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
            view->setDragDropOverwriteMode(false);
            view->setDropIndicatorShown(true);

            ui::typical_tableview_config(view);
            view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
            ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
               header.setColumnFlex(0, 0, 0, metrics.boundingRect("Facial Hair").width() * 1.5F + 4);
               header.setColumnFlex(1, 1, 0);
            });

            auto* filter = this->_filters.base_head_part[sex] = new HeadPartPickerFilter(this);
            picker->setAllowedFormType(dovah::form_type::head_part);
            picker->setCustomFilter(filter);
            filter->setRequiredRace(this->formStub());
            filter->setRequiredSex(sex);

            auto* sel_model = view->selectionModel();
            QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, view, model, sel_model, picker, filter]() {
               std::optional<FaceBaseHeadPartsModel::Slot> slot;
               {
                  auto rows = sel_model->selectedRows();
                  if (!rows.isEmpty())
                     slot = model->slotAt(rows[0].row());
               }
               if (!slot.has_value()) {
                  picker->setEnabled(false);
                  return;
               }

               picker->setEnabled(true);
               auto* stub = model->headPartFor(slot.value());

               auto blocker = QSignalBlocker(picker);
               filter->setRequiredType(FaceBaseHeadPartsModel::slotToType(slot.value()));
               picker->setFormStub(stub);
            });
            QObject::connect(picker, &DKFormPicker::formChanged, this, [this, sel_model, model, picker](dovah::form_stub* stub) {
               std::optional<FaceBaseHeadPartsModel::Slot> slot;
               {
                  auto rows = sel_model->selectedRows();
                  if (!rows.isEmpty())
                     slot = model->slotAt(rows[0].row());
               }
               if (!slot.has_value())
                  return;

               model->setHeadPartFor(slot.value(), stub);
            });
         };
         _configure(dovah::sex::female, this->ui.baseHeadPartsF, this->ui.baseHeadPartPickerF);
         _configure(dovah::sex::male,   this->ui.baseHeadPartsM, this->ui.baseHeadPartPickerM);
      }
      {  // Additional Head Parts
         auto _configure = [this](dovah::sex sex, QTableView* view) {
            this->_subwidgets.head_parts.extra[sex] = view;

            auto* model = this->_models.head_parts_extra[sex] = new FaceExtraHeadPartsModel(this);
            view->setModel(model);
            view->setAcceptDrops(true);
            view->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
            view->setDragDropOverwriteMode(false);
            view->setDropIndicatorShown(true);

            ui::typical_tableview_config(view);
            view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
            ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
               header.setColumnFlex(0, 0, 0, metrics.boundingRect("Facial Hair").width() * 1.5F + 4);
               header.setColumnFlex(1, 1, 0);
            });

            // Handle the Del key for removing head parts from the list.
            view->installEventFilter(this);
         };
         _configure(dovah::sex::female, this->ui.extraHeadPartsF);
         _configure(dovah::sex::male,   this->ui.extraHeadPartsM);
      }
      static_assert(false, "TODO: Available Morphs");
      this->ui.hairColorsF->setAllowedFormTypes({ dovah::form_type::color });
      this->ui.hairColorsM->setAllowedFormTypes({ dovah::form_type::color });
      this->ui.defaultHairColorF->setAllowedFormType(dovah::form_type::color);
      this->ui.defaultHairColorM->setAllowedFormType(dovah::form_type::color);
   #pragma endregion
   #pragma region Face Tints tab
      static_assert(false, "TODO: Tint layers and editing thereof");
      this->ui.currentTintFDefaultColor->setAllowedFormType(dovah::form_type::color);
      this->ui.currentTintMDefaultColor->setAllowedFormType(dovah::form_type::color);
      this->ui.defaultFaceTextureF->setAllowedFormType(dovah::form_type::texture_set);
      this->ui.defaultFaceTextureM->setAllowedFormType(dovah::form_type::texture_set);
      this->ui.faceTexturesF->setAllowedFormTypes({ dovah::form_type::texture_set });
      this->ui.faceTexturesM->setAllowedFormTypes({ dovah::form_type::texture_set });
   #pragma endregion
   #pragma region Presets tab
      this->ui.presetsF->setAllowedFormTypes({ dovah::form_type::actor_base });
      this->ui.presetsM->setAllowedFormTypes({ dovah::form_type::actor_base });
      static_assert(false, "TODO: Filter female list to female ActorBases of this race");
      static_assert(false, "TODO: Filter male list to male ActorBases of this race");
   #pragma endregion

   this->load(); // this creates the working copy.
}
void FormDialogRace::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   
   #pragma region General tab
      // Inherit
      ui::bind(this->ui.morphRace, working.morph_race, working);
      ui::bind(this->ui.armorRace, working.armor_race, working);

      // Miscellaneous
      ui::bind(this->ui.creatureSize,      working.stats.creature_size);
      ui::bind(this->ui.flightRadius,      working.movement.flight_radius);
      ui::bind(this->ui.baseCarryCapacity, working.stats.base_carry_capacity);

      // Flags
      {
         using flag = loaded_form_type::race_flag;
         auto& dst  = working.race_flags;

         ui::bind(this->ui.flagPlayable,        dst, flag::playable);
         ui::bind(this->ui.flagAllowPickpocket, dst, flag::can_be_pickpocketed);
         ui::bind(this->ui.flagNoKnockdowns,    dst, flag::no_knockdowns);

         ui::bind(this->ui.flagChild,           dst, flag::child);
         ui::bind(this->ui.flagCannotOpenDoors, dst, flag::cant_open_doors);
         ui::bind(this->ui.flagNoShadow,        dst, flag::no_shadow);

         ui::bind(this->ui.flagAllowPlayerDialogue,      dst, flag::allow_player_dialogue);
         ui::bind(this->ui.flagAllowRagdollCollision,    dst, flag::allow_ragdoll_collision);
         ui::bind(this->ui.flagSpellsAlignWithMagicNode, dst, flag::spells_align_with_magic_node);

         ui::bind(this->ui.flagFaceGenHead,             dst, flag::facegen_head);
         ui::bind(this->ui.flagMultipleMembraneShaders, dst, flag::allow_multiple_membrane_shaders);
         ui::bind(this->ui.flagCanPickUpItems,          dst, flag::can_pick_up_items);
      }

      // Skill bonuses
      for (size_t i = 0; i < num_skill_boosts; ++i) {
         auto* widget_which = this->_subwidgets.skills.which[i];
         auto* widget_value = this->_subwidgets.skills.boost[i];

         auto& src_opt = working.stats.skill_boosts[i];
         if (src_opt.has_value()) {
            auto& src = src_opt.value();
            widget_which->setCurrentIndex(widget_which->findData((int)src.skill));
            widget_value->setValue(src.boost);
         } else {
            widget_which->setCurrentIndex(widget_which->findData(-1));
            widget_value->setValue(0);
         }
      }

      // Attributes
      ui::bind(this->ui.attrBaseH, working.stats.attribute_base.h);
      ui::bind(this->ui.attrBaseM, working.stats.attribute_base.m);
      ui::bind(this->ui.attrBaseS, working.stats.attribute_base.s);
      ui::bind(this->ui.attrRegenH, working.stats.attribute_regen.h);
      ui::bind(this->ui.attrRegenM, working.stats.attribute_regen.m);
      ui::bind(this->ui.attrRegenS, working.stats.attribute_regen.s);
      ui::bind(this->ui.flagRegenInCombat, working.race_flags, loaded_form_type::race_flag::regen_health_in_combat);

      // Spells
      this->ui.abilities->pullStubs(working.spells.forms);

      // Basic Movement Options
      {
         using flag = loaded_form_type::race_flag;
         auto& dst  = working.race_flags;

         ui::bind(this->ui.flagImmobile, dst, flag::immobile);
         ui::bind(this->ui.flagWalks, dst, flag::walks);
         ui::bind(this->ui.flagSwims, dst, flag::swims);
         ui::bind(this->ui.flagFlies, dst, flag::flies);
         ui::bind(this->ui.flagAvoidsRoads, dst, flag::avoids_roads);
         ui::bind(this->ui.flagNotPushable, dst, flag::not_pushable);
         ui::bind(this->ui.flagNoWaterCombat, dst, flag::no_water_combat);
         ui::bind(this->ui.flagNoRotateToHeadtrack, dst, flag::no_rotate_to_headtrack);
         ui::bind(this->ui.flagUsesHeadtrackAnims, dst, flag::uses_headtrack_anims);

         ui::bind(this->ui.flagTiltPitch, dst, flag::tilt_front_back);
         ui::bind(this->ui.flagTiltRoll,  dst, flag::tilt_left_right);
         ui::bind(this->ui.flagWorldRaycastsForIK,    dst, flag::use_world_raycasts_for_foot_ik);
         ui::bind(this->ui.flagAlwaysProxyController, dst, flag::always_use_proxy_controller);
      }

      // Mount Data
      ui::bind(this->ui.mountOffsetX, working.mount_data.climb_on_offset.x);
      ui::bind(this->ui.mountOffsetY, working.mount_data.climb_on_offset.y);
      ui::bind(this->ui.mountOffsetZ, working.mount_data.climb_on_offset.z);
      ui::bind(this->ui.dismountOffsetX, working.mount_data.dismount_offset.x);
      ui::bind(this->ui.dismountOffsetY, working.mount_data.dismount_offset.y);
      ui::bind(this->ui.dismountOffsetZ, working.mount_data.dismount_offset.z);
      ui::bind(this->ui.mountCamOffsetX, working.mount_data.camera_offset.x);
      ui::bind(this->ui.mountCamOffsetY, working.mount_data.camera_offset.y);
      ui::bind(this->ui.mountCamOffsetZ, working.mount_data.camera_offset.z);
      ui::bind(this->ui.flagAllowMountedCombat, working.alt_flags, loaded_form_type::alt_flag::allow_mounted_combat);
   #pragma endregion
   #pragma region Body tab
      ui::bind(this->ui.mass, working.stats.base_mass);
      ui::bind(this->ui.bodyPartData, working.body_part_data, working);
      ui::bind(this->ui.skin, working.skin, working);

      {  // Female
         auto& dst = working.by_sex.female;
         ui::bind(this->ui.heightMultF, dst.height_mult);
         ui::bind(this->ui.bodyWeightF, dst.weight);
         this->ui.skeletonF->initializeFrom(dst.skeleton_nif);
         this->ui.behaviorGraphF->initializeFrom(dst.behavior_graph);
         this->ui.bodyTextureF->initializeFrom(dst.lighting_model);
         ui::bind(this->ui.decapArmorF, dst.decapitate_armor, working);
         ui::bind(this->ui.voicetypeF, dst.voicetype, working);
      }
      {  // Male
         auto& dst = working.by_sex.male;
         ui::bind(this->ui.heightMultM, dst.height_mult);
         ui::bind(this->ui.bodyWeightM, dst.weight);
         this->ui.skeletonM->initializeFrom(dst.skeleton_nif);
         this->ui.behaviorGraphM->initializeFrom(dst.behavior_graph);
         this->ui.bodyTextureM->initializeFrom(dst.lighting_model);
         ui::bind(this->ui.decapArmorM, dst.decapitate_armor, working);
         ui::bind(this->ui.voicetypeM, dst.voicetype, working);
      }

      static_assert(false, "TODO: Body Slot");
      static_assert(false, "TODO: Hair Slot");
      static_assert(false, "TODO: Head Slot");
      static_assert(false, "TODO: Shield Slot");
      static_assert(false, "TODO: Slot Names");
      static_assert(false, "TODO: Visible in First Person");
   #pragma endregion
   #pragma region Blood tab
      ui::bind(this->ui.impactMaterialType, working.material_type, working);
      ui::bind(this->ui.decapBloodArt, working.decapitation_effect, working);
      ui::bind(this->ui.impactDataSet, working.impact_data_set, working);
      ui::bind(this->ui.soundOpen, working.container_sounds.open, working);
      ui::bind(this->ui.soundClose, working.container_sounds.close, working);
   #pragma endregion
   #pragma region Text tab
      this->ui.name->setText(editor.convert_localized_string(working.name));
      this->ui.description->setPlainText(editor.convert_localized_string(working.description));
   #pragma endregion
   #pragma region Movement Details tab
      ui::bind(this->ui.accelerationRate, working.movement.acceleration_rate);
      ui::bind(this->ui.decelerationRate, working.movement.deceleration_rate);
      ui::bind(this->ui.angularAccelerationRate, working.movement.angular_acceleration_rate);
      ui::bind(this->ui.angularTolerance, working.movement.angular_tolerance);
      ui::bind(this->ui.flagUseAdvancedAvoidance, working.alt_flags, loaded_form_type::alt_flag::use_advanced_avoidance);

      static_assert(false, "TODO: Base Movement Defaults");
      static_assert(false, "TODO: Movement Data Overrides");
   #pragma endregion
   #pragma region Attack Data tab
      this->ui.attackData->initializeFrom(working.attack_data);
   #pragma endregion
   #pragma region Combat tab
      ui::bind(this->ui.injuredHealthPercentage, working.stats.injured_health_threshold);
      ui::bind(this->ui.unarmedDamage, working.stats.unarmed.damage);
      ui::bind(this->ui.unarmedReach, working.stats.unarmed.reach);
      ui::bind(this->ui.unarmedEquipSlot, working.equipment.unarmed_equip_slot, working);
      ui::bind(this->ui.aimAngleTolerance, working.stats.aim_angle_tolerance);
      ui::bind(this->ui.flagCanDualWield, working.race_flags, loaded_form_type::race_flag::can_dual_wield);
      ui::bind(this->ui.flagNonHostile, working.alt_flags, loaded_form_type::alt_flag::non_hostile);

      static_assert(false, "TODO: Equipment Restrictions");
   #pragma endregion
   #pragma region Lip Synching tab
      static_assert(false, "TODO: FaceFX Phonemes");
      static_assert(false, "TODO: Phoneme Targets");
      static_assert(false, "TODO: Default FaceGen Targets and Weights");
   #pragma endregion
   #pragma region Face Data tab
      {
         this->ui.faceHeadPartsOverlay->setProperty("inherit_flag", (int)loaded_form_type::race_flag::overlay_head_part_list);
         this->ui.faceHeadPartsOverride->setProperty("inherit_flag", (int)loaded_form_type::race_flag::override_head_part_list);
         this->ui.faceHeadPartsInherit->setProperty("inherit_flag", (int)0);
         auto& widgets = this->_subwidgets.head_part_inheritance.all = {
            this->ui.faceHeadPartsOverlay,
            this->ui.faceHeadPartsOverride,
            this->ui.faceHeadPartsInherit,
         };

         for (auto* widget : widgets) {
            if (working.race_flags & widget->property("inherit_flag").toInt()) {
               widget->setChecked(true);
               break;
            }
         }
         for (auto* widget : widgets) {
            QObject::connect(widget, &QRadioButton::toggled, this, [this, widget](bool checked) {
               constexpr const auto all_flags = loaded_form_type::race_flag::overlay_head_part_list | loaded_form_type::race_flag::override_head_part_list;

               if (!checked)
                  return;
               auto& dst = this->form->race_flags;
               dst &= ~all_flags;
               dst |= (loaded_form_type::race_flags_t) widget->property("inherit_flag").toInt();
            });
         }
      }
      {  // Base Head Parts and Additional Head Parts
         using base_slot = FaceBaseHeadPartsModel::Slot;

         auto& fic = dovahkit::subsystems::form_info_cache::core::get();
         for (size_t i = 0; i < dovah::sex_count; ++i) {
            auto sex = (dovah::sex)i;
         
            auto* model_base  = this->_models.head_parts_base[sex];
            auto* model_extra = this->_models.head_parts_extra[sex];

            std::vector<dovah::form_stub*> extra_parts;

            // Used to strip out duplicates on load.
            auto _head_part_already_seen = [&extra_parts, model_base](const dovah::form_stub* stub) {
               if (stub == model_base->headPartFor(base_slot::Brows))
                  return true;
               if (stub == model_base->headPartFor(base_slot::Eyes))
                  return true;
               if (stub == model_base->headPartFor(base_slot::Face))
                  return true;
               if (stub == model_base->headPartFor(base_slot::FacialHair))
                  return true;
               if (stub == model_base->headPartFor(base_slot::Hair))
                  return true;

               auto it = std::find(extra_parts.begin(), extra_parts.end(), stub);
               if (it != extra_parts.end())
                  return true;

               return false;
            };

            for (auto& form_use : working.by_sex[sex].head_data.head_parts) {
               auto* stub = form_use.get_form_stub();
               if (stub && stub->form_type == dovah::form_type::head_part) {
                  if (_head_part_already_seen(stub))
                     continue;
                  auto* info = fic.get_head_part_info(*stub);
                  if (!info) {
                     extra_parts.push_back(stub);
                     continue;
                  }
                  switch (info->type) {
                     case dovah::head_part_type::eyebrows:
                        model_base->setHeadPartFor(base_slot::Brows, stub);
                        continue;
                     case dovah::head_part_type::eyes:
                        model_base->setHeadPartFor(base_slot::Eyes, stub);
                        continue;
                     case dovah::head_part_type::face:
                        model_base->setHeadPartFor(base_slot::Face, stub);
                        continue;
                     case dovah::head_part_type::facial_hair:
                        model_base->setHeadPartFor(base_slot::FacialHair, stub);
                        continue;
                     case dovah::head_part_type::hair:
                        model_base->setHeadPartFor(base_slot::Hair, stub);
                        continue;
                  }
                  extra_parts.push_back(stub);
               }
            }
            model_extra->replaceAllHeadParts(extra_parts);
         }
      }
      static_assert(false, "TODO: Female: Morphs");
      static_assert(false, "TODO: Male: Morphs");
      this->ui.hairColorsF->pullStubs(working.by_sex.female.head_data.hair_colors);
      this->ui.hairColorsM->pullStubs(working.by_sex.male.head_data.hair_colors);
      ui::bind(this->ui.defaultHairColorF, working.by_sex.female.head_data.default_hair_color, working);
      ui::bind(this->ui.defaultHairColorM, working.by_sex.male.head_data.default_hair_color, working);
   #pragma endregion
   #pragma region Face Tints tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Presets tab
      this->ui.presetsF->pullStubs(working.by_sex.female.head_data.preset_actors);
      this->ui.presetsM->pullStubs(working.by_sex.male.head_data.preset_actors);
   #pragma endregion
}
void FormDialogRace::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   #pragma region General tab
      // Skill bonuses
      for (size_t i = 0; i < num_skill_boosts; ++i) {
         auto* widget_which = this->_subwidgets.skills.which[i];
         auto* widget_value = this->_subwidgets.skills.boost[i];

         auto& dst_opt = working.stats.skill_boosts[i];

         auto data = widget_which->currentData().toInt();
         if (data < 0) {
            dst_opt = {};
         } else {
            auto& dst = dst_opt.emplace();
            dst.skill = (dovah::skill)data;
            dst.boost = widget_value->value();
         }
      }

      // Spells
      this->ui.abilities->commitStubs(working.spells.forms, working);
   #pragma endregion
   #pragma region Body tab
      {  // Female
         auto& dst = working.by_sex.female;
         this->ui.skeletonF->commitTo(dst.skeleton_nif, working);
         this->ui.behaviorGraphF->commitTo(dst.behavior_graph, working);
         this->ui.bodyTextureF->commitTo(dst.lighting_model, working);
      }
      {  // Male
         auto& dst = working.by_sex.male;
         this->ui.skeletonM->commitTo(dst.skeleton_nif, working);
         this->ui.behaviorGraphM->commitTo(dst.behavior_graph, working);
         this->ui.bodyTextureM->commitTo(dst.lighting_model, working);
      }

      static_assert(false, "TODO: Body Slot");
      static_assert(false, "TODO: Hair Slot");
      static_assert(false, "TODO: Head Slot");
      static_assert(false, "TODO: Shield Slot");
      static_assert(false, "TODO: Slot Names");
      static_assert(false, "TODO: Visible in First Person");
   #pragma endregion
   #pragma region Blood tab
      //
      // All properties in here are live-updated.
      //
   #pragma endregion
   #pragma region Text tab
      editor.assign_localized_string(working.name,        this->ui.name->text());
      editor.assign_localized_string(working.description, this->ui.description->toPlainText());
   #pragma endregion
   #pragma region Movement Details tab
      static_assert(false, "TODO: Base Movement Defaults");
      static_assert(false, "TODO: Movement Data Overrides");
   #pragma endregion
   #pragma region Attack Data tab
      this->ui.attackData->commitTo(working.attack_data, working);
   #pragma endregion
   #pragma region Combat tab
      static_assert(false, "TODO: Equipment Restrictions");
   #pragma endregion
   #pragma region Lip Synching tab
      static_assert(false, "TODO: FaceFX Phonemes");
      static_assert(false, "TODO: Phoneme Targets");
      static_assert(false, "TODO: Default FaceGen Targets and Weights");
   #pragma endregion
   #pragma region Face Data tab
      // Base Head Parts and Additional Head Parts
      for (size_t i = 0; i < dovah::sex_count; ++i) {
         auto sex = (dovah::sex)i;
         
         std::vector<dovah::form_stub*> head_parts;
         {
            auto* model = this->_models.head_parts_base[sex];
            if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Brows))
               head_parts.push_back(stub);
            if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Eyes))
               head_parts.push_back(stub);
            if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Face))
               head_parts.push_back(stub);
            if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::FacialHair))
               head_parts.push_back(stub);
            if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Hair))
               head_parts.push_back(stub);
         }
         {
            auto*  model = this->_models.head_parts_extra[sex];
            size_t size  = model->rowCount();
            head_parts.reserve(head_parts.size() + size);
            for (size_t i = 0; i < size; ++i) {
               if (auto* stub = model->headPart(i))
                  head_parts.push_back(stub);
            }
         }

         auto& dst_list = working.by_sex[sex].head_data.head_parts;
         dovah::clear_form_reference_list(dst_list, working);
         for (auto* stub : head_parts) {
            auto& form_use = dst_list.emplace_back();
            write_form_ref(form_use, stub);
         }
      }

      static_assert(false, "TODO: Female: Morphs");
      static_assert(false, "TODO: Male: Morphs");

      this->ui.hairColorsF->commitStubs(working.by_sex.female.head_data.hair_colors, working);
      this->ui.hairColorsM->commitStubs(working.by_sex.male.head_data.hair_colors, working);
   #pragma endregion
   #pragma region Face Tints tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Presets tab
      this->ui.presetsF->commitStubs(working.by_sex.female.head_data.preset_actors, working);
      this->ui.presetsM->commitStubs(working.by_sex.male.head_data.preset_actors, working);
   #pragma endregion
}

/*virtual*/ bool FormDialogRace::eventFilter(QObject* object, QEvent* event) /*override*/ {
   //
   // Handle the Delete key on the Additional Head Parts listviews:
   //
   for (size_t i = 0; i < dovah::sex_count; ++i) {
      auto  sex    = (dovah::sex)i;
      auto* widget = this->_subwidgets.head_parts.extra[sex];
      if (object == widget) {
         if (event->type() == QEvent::Type::KeyPress) {
            auto* casted = (QKeyEvent*)event;
            if (casted->key() == Qt::Key_Delete) {

               auto* sel_model = widget->selectionModel();
               auto* model     = this->_models.head_parts_extra[sex];
               {
                  auto rows = sel_model->selectedRows();
                  if (!rows.isEmpty()) {
                     auto  row  = rows[0].row();
                     auto* stub = model->headPart(row);
                     if (stub)
                        model->removeHeadPart(*stub);
                  }
               }

               return true;
            }
            return false;
         }
         break;
      }
   }
   return false;
}