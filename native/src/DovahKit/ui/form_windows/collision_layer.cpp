#include "./collision_layer.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogCollisionLayer::FormDialogCollisionLayer(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.name->setMaxLength(loaded_form_type::max_name_length);
   this->ui.layerID->setMaximum(std::numeric_limits<float>::max());
   this->ui.collidesWith->setAllowedFormTypes({ dovah::form_type::collision_layer });

   this->load(); // this creates the working copy.
}
void FormDialogCollisionLayer::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.name, working.name);
   ui::bind(this->ui.layerID, working.unique_id);
   this->ui.description->setPlainText(editor.convert_localized_string(working.description));
   ui::bind(this->ui.color, working.debug_color);
   this->ui.collidesWith->pullStubs(working.collides_with);
   ui::bind(this->ui.flagSensor,          working.layer_flags, loaded_form_type::layer_flag::sensor);
   ui::bind(this->ui.flagTriggerVolume,   working.layer_flags, loaded_form_type::layer_flag::trigger_volume);
   ui::bind(this->ui.flagNavmeshObstacle, working.layer_flags, loaded_form_type::layer_flag::navmesh_obstacle);
}
void FormDialogCollisionLayer::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.description, this->ui.description->toPlainText());
   this->ui.collidesWith->commitStubs(working.collides_with, working);
}