#include "./visual_effect.h"
#include <limits>
#include "ui/utils/bind.h"

FormDialogVisualEffect::FormDialogVisualEffect(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.artObject->setAllowedFormType(dovah::form_type::art_object);
   this->ui.effectShader->setAllowedFormType(dovah::form_type::effect_shader);

   QObject::connect(this->ui.flagAttachToCamera, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.flagFaceTarget->setEnabled(!checked);
      this->ui.flagInheritRotation->setEnabled(checked);
   });

   this->load(); // this creates the working copy.
}
void FormDialogVisualEffect::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.artObject,    working.art_object, working);
   ui::bind(this->ui.effectShader, working.effect_shader, working);
   ui::bind(this->ui.flagFaceTarget,      working.flags.rotate_to_face_target);
   ui::bind(this->ui.flagAttachToCamera,  working.flags.camera_attached);
   ui::bind(this->ui.flagInheritRotation, working.flags.inherit_rotation);
}
void FormDialogVisualEffect::_save_impl() {
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