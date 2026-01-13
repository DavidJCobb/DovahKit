#include "./combat_style.h"
#include <limits>
#include "ui/utils/bind.h"
#include "ui/utils/pair_slider_to_spinbox.h"

FormDialogCombatStyle::FormDialogCombatStyle(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   #define SETUP_PREC_SLIDER(name) ui::pair_slider_to_spinbox(this->ui.name##Slider, this->ui.name##Spinbox);
   // General
   SETUP_PREC_SLIDER(avoidThreatChance);
   SETUP_PREC_SLIDER(defenseMult);
   SETUP_PREC_SLIDER(groupOffenseMult);
   SETUP_PREC_SLIDER(offenseMult);
   SETUP_PREC_SLIDER(equipScoreMagic);
   SETUP_PREC_SLIDER(equipScoreMelee);
   SETUP_PREC_SLIDER(equipScoreRanged);
   SETUP_PREC_SLIDER(equipScoreShout);
   SETUP_PREC_SLIDER(equipScoreStaff);
   SETUP_PREC_SLIDER(equipScoreUnarmed);
   // Melee
   SETUP_PREC_SLIDER(meleeAttackStaggeredMult);
   SETUP_PREC_SLIDER(meleePowerAttackBlockingMult);
   SETUP_PREC_SLIDER(meleePowerAttackStaggeredMult);
   SETUP_PREC_SLIDER(meleeSpecialAttackMult);
   SETUP_PREC_SLIDER(bashAttackMult);
   SETUP_PREC_SLIDER(bashMult);
   SETUP_PREC_SLIDER(bashPowerAttackMult);
   SETUP_PREC_SLIDER(bashRecoiledMult);
   // Close Range
   SETUP_PREC_SLIDER(closeRangeDuelCircleMult);
   SETUP_PREC_SLIDER(closeRangeDuelFallbackMult);
   SETUP_PREC_SLIDER(closeRangeFlankDistance);
   SETUP_PREC_SLIDER(closeRangeFlankStalkTime);
   // Long Range
   SETUP_PREC_SLIDER(longRangeStrafeMult);
   // Flight
   SETUP_PREC_SLIDER(flightDivebombChance);
   SETUP_PREC_SLIDER(flightFlyingAttackChance);
   SETUP_PREC_SLIDER(flightGroundAttackChance);
   SETUP_PREC_SLIDER(flightGroundAttackTime);
   SETUP_PREC_SLIDER(flightHoverChance);
   SETUP_PREC_SLIDER(flightHoverTime);
   SETUP_PREC_SLIDER(flightPerchAttackChance);
   SETUP_PREC_SLIDER(flightPerchAttackTime);
   #undef SETUP_PREC_SLIDER

   this->load(); // this creates the working copy.
}
void FormDialogCombatStyle::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {
      auto& dst = working.general;
      ui::bind(this->ui.avoidThreatChanceSpinbox, dst.avoid_threat_chance);
      ui::bind(this->ui.defenseMultSlider, dst.defensive_mult);
      ui::bind(this->ui.offenseMultSpinbox, dst.offensive_mult);
      ui::bind(this->ui.groupOffenseMultSpinbox, dst.group_offensive_mult);
      {
         auto& dst = working.general.equipment_score_mults;
         ui::bind(this->ui.equipScoreMagicSpinbox, dst.magic);
         ui::bind(this->ui.equipScoreMeleeSpinbox, dst.melee);
         ui::bind(this->ui.equipScoreRangedSpinbox, dst.ranged);
         ui::bind(this->ui.equipScoreShoutSpinbox, dst.shout);
         ui::bind(this->ui.equipScoreStaffSpinbox, dst.staff);
         ui::bind(this->ui.equipScoreUnarmedSpinbox, dst.unarmed);
      }
   }
   {
      auto& dst = working.melee;
      ui::bind(this->ui.meleeAttackStaggeredMultSpinbox, dst.attack_staggered_mult);
      ui::bind(this->ui.meleePowerAttackBlockingMultSpinbox, dst.power_attack_blocking_mult);
      ui::bind(this->ui.meleePowerAttackStaggeredMultSpinbox, dst.power_attack_staggered_mult);
      ui::bind(this->ui.meleeSpecialAttackMultSpinbox, dst.special_attack_mult);
      ui::bind(this->ui.bashAttackMultSpinbox, dst.bash_attack_mult);
      ui::bind(this->ui.bashMultSpinbox, dst.bash_mult);
      ui::bind(this->ui.bashPowerAttackMultSpinbox, dst.bash_power_attack_mult);
      ui::bind(this->ui.bashRecoiledMultSpinbox, dst.bash_recoil_mult);
      ui::bind(this->ui.flagAllowDualWield, working.flags, loaded_form_type::flag::allow_dual_wielding);
   }
   {
      auto& dst = working.close_range;
      {
         this->ui.closeRangeUseDueling->setChecked((working.flags & loaded_form_type::flag::dueling) != 0);
         this->ui.closeRangeUseFlanking->setChecked((working.flags & loaded_form_type::flag::flanking) != 0);
         QObject::connect(this->ui.closeRangeUseDueling, &QRadioButton::toggled, this, [this](bool checked) {
            cobb::edit_bit(this->form->flags, loaded_form_type::flag::dueling,  checked);
            cobb::edit_bit(this->form->flags, loaded_form_type::flag::flanking, !checked);
         });
         QObject::connect(this->ui.closeRangeUseFlanking, &QRadioButton::toggled, this, [this](bool checked) {
            cobb::edit_bit(this->form->flags, loaded_form_type::flag::dueling,  !checked);
            cobb::edit_bit(this->form->flags, loaded_form_type::flag::flanking, checked);
         });
      }
      ui::bind(this->ui.closeRangeDuelCircleMultSpinbox, dst.circle_mult);
      ui::bind(this->ui.closeRangeDuelFallbackMultSpinbox, dst.fallback_mult);
      ui::bind(this->ui.closeRangeFlankDistanceSpinbox, dst.flank_distance);
      ui::bind(this->ui.closeRangeFlankStalkTimeSpinbox, dst.stalk_time);
   }
   {
      auto& dst = working.long_range;
      ui::bind(this->ui.longRangeStrafeMultSpinbox, dst.strafe_mult);
   }
   {
      auto& dst = working.flight;
      ui::bind(this->ui.flightDivebombChanceSpinbox, dst.divebomb.chance);
      ui::bind(this->ui.flightFlyingAttackChanceSpinbox, dst.flying_attack.chance);
      ui::bind(this->ui.flightGroundAttackChanceSpinbox, dst.ground_attack.chance);
      ui::bind(this->ui.flightGroundAttackTimeSpinbox, dst.ground_attack.time);
      ui::bind(this->ui.flightHoverChanceSpinbox, dst.hover.chance);
      ui::bind(this->ui.flightHoverTimeSpinbox, dst.hover.time);
      ui::bind(this->ui.flightPerchAttackChanceSpinbox, dst.perch_attack.chance);
      ui::bind(this->ui.flightPerchAttackTimeSpinbox, dst.perch_attack.time);
   }
}
void FormDialogCombatStyle::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   cobb::edit_bit(this->record_flags(), loaded_form_type::form_flag::allow_dual_wielding, (working.flags & loaded_form_type::flag::allow_dual_wielding));
}