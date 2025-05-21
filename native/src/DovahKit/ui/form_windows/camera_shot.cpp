#include "./camera_shot.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"

FormDialogCameraShot::FormDialogCameraShot(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   for (auto* widget : std::array{
      this->ui.location,
      this->ui.target,
   }) {
      using enumeration = loaded_form_type::camera_subject;
      widget->clear();
      widget->addItem(tr("Attacker"), (int)enumeration::attacker);
      widget->addItem(tr("Projectile"), (int)enumeration::projectile);
      widget->addItem(tr("Target"), (int)enumeration::target);
      widget->addItem(tr("Lead Actor"), (int)enumeration::lead_actor);
      widget->model()->sort(0);
   }
   {
      using enumeration = loaded_form_type::camera_action;
      auto* widget = this->ui.action;
      widget->clear();
      widget->addItem(tr("Fly"),   (int)enumeration::fly);
      widget->addItem(tr("Hit"),   (int)enumeration::hit);
      widget->addItem(tr("Shoot"), (int)enumeration::shoot);
      widget->addItem(tr("Zoom"),  (int)enumeration::zoom);
   }

   this->ui.imagespaceMod->setAllowedFormType(dovah::form_type::imagespace_modifier);

   this->load(); // this creates the working copy.
}
void FormDialogCameraShot::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.location, working.location);
   ui::bind(this->ui.action, working.action);
   ui::bind(this->ui.target, working.target);
   {
      using flag = loaded_form_type::camera_shot_flag;
      ui::bind(this->ui.flagPosFollowsLoc,    working.flags, flag::position_follows_location);
      ui::bind(this->ui.flagDoNotFollowBone,  working.flags, flag::do_not_follow_bone);
      ui::bind(this->ui.flagStartAtTimeZero,  working.flags, flag::start_at_time_zero);
      ui::bind(this->ui.flagRotFollowsTarget, working.flags, flag::rotation_follows_target);
      ui::bind(this->ui.flagFirstPerson,      working.flags, flag::first_person_camera);
      ui::bind(this->ui.flagNoTracer,         working.flags, flag::no_tracer);
   }
   ui::bind(this->ui.timeMultGlobal, working.time_multipliers.global);
   ui::bind(this->ui.timeMultPlayer, working.time_multipliers.player);
   ui::bind(this->ui.timeMultTarget, working.time_multipliers.target);
   ui::bind(this->ui.timeMin, working.minimum_time);
   ui::bind(this->ui.timeMax, working.maximum_time);
   ui::bind(this->ui.targetPct, working.target_percentage_between_actors);
   ui::bind(this->ui.nearTargetDistance, working.near_target_distance);
   ui::bind(this->ui.imagespaceMod, working.imagespace_modifier, working);
}
void FormDialogCameraShot::_save_impl() {
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
}