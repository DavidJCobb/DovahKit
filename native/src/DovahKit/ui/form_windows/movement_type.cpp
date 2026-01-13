#include "./movement_type.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogMovementType::FormDialogMovementType(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::set_unsigned_range<float>(this->ui.leftWalk);
   ui::set_unsigned_range<float>(this->ui.leftRun);
   ui::set_unsigned_range<float>(this->ui.rightRun);
   ui::set_unsigned_range<float>(this->ui.rightWalk);
   ui::set_unsigned_range<float>(this->ui.forwardRun);
   ui::set_unsigned_range<float>(this->ui.forwardWalk);
   ui::set_unsigned_range<float>(this->ui.backRun);
   ui::set_unsigned_range<float>(this->ui.backWalk);
   ui::set_unsigned_range<float>(this->ui.rotateInPlaceWalk);
   ui::set_unsigned_range<float>(this->ui.rotateInPlaceRun);
   ui::set_unsigned_range<float>(this->ui.rotateMovingRun);

   QObject::connect(this->ui.animDirChangeFlag, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.animDirChangeValue->setEnabled(checked);
   });
   QObject::connect(this->ui.animMoveChangeFlag, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.animMoveChangeValue->setEnabled(checked);
   });
   QObject::connect(this->ui.animRotChangeFlag, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.animRotChangeValue->setEnabled(checked);
   });
   ui::set_unsigned_range<float>(this->ui.animDirChangeValue);
   ui::set_unsigned_range<float>(this->ui.animMoveChangeValue);
   ui::set_unsigned_range<float>(this->ui.animRotChangeValue);

   this->load(); // this creates the working copy.
}
void FormDialogMovementType::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.name, working.name);
   //
   ui::bind(this->ui.leftWalk, working.speeds.left.walk);
   ui::bind(this->ui.leftRun, working.speeds.left.run);
   ui::bind(this->ui.rightWalk, working.speeds.right.walk);
   ui::bind(this->ui.rightRun, working.speeds.right.run);
   ui::bind(this->ui.forwardWalk, working.speeds.forward.walk);
   ui::bind(this->ui.forwardRun, working.speeds.forward.run);
   ui::bind(this->ui.backWalk, working.speeds.back.walk);
   ui::bind(this->ui.backRun, working.speeds.back.run);
   ui::bind(this->ui.rotateInPlaceWalk, working.speeds.rotate_in_place.walk);
   ui::bind(this->ui.rotateInPlaceRun, working.speeds.rotate_in_place.run);
   ui::bind(this->ui.rotateMovingRun, working.speeds.rotate_while_moving);
   //
   ui::bind(this->ui.animDirChangeValue, working.anim_change_thresholds.directional);
   ui::bind(this->ui.animMoveChangeValue, working.anim_change_thresholds.movement_speed);
   ui::bind(this->ui.animRotChangeValue, working.anim_change_thresholds.rotation_speed);
   {
      auto _handle = [](float& src, QCheckBox* flag, QDoubleSpinBox* value) {
         if (src == loaded_form_type::anim_change_threshold_disabled) {
            const auto blocker = QSignalBlocker(value);

            flag->setChecked(false);
            value->setValue(0);
         } else {
            flag->setChecked(true);
         }
      };
      _handle(working.anim_change_thresholds.directional,    this->ui.animDirChangeFlag,  this->ui.animDirChangeValue);
      _handle(working.anim_change_thresholds.movement_speed, this->ui.animMoveChangeFlag, this->ui.animMoveChangeValue);
      _handle(working.anim_change_thresholds.rotation_speed, this->ui.animRotChangeFlag,  this->ui.animRotChangeValue);
   }
}
void FormDialogMovementType::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& working = *this->form;
   if (!this->ui.animDirChangeFlag->isChecked()) {
      working.anim_change_thresholds.directional = loaded_form_type::anim_change_threshold_disabled;
   }
   if (!this->ui.animMoveChangeFlag->isChecked()) {
      working.anim_change_thresholds.movement_speed = loaded_form_type::anim_change_threshold_disabled;
   }
   if (!this->ui.animRotChangeFlag->isChecked()) {
      working.anim_change_thresholds.rotation_speed = loaded_form_type::anim_change_threshold_disabled;
   }
}