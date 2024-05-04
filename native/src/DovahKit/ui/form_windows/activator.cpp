#include "./activator.h"
#include "dovah/core.h"
#include "dovah/form_stub_addenda.h"
#include "helpers/qt/basic_bindings.h"
#include "widgets/DKFormNIFPicker.h"

#include "ui/utils/bind.h"

#include "../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "The form-editing dialog for Activators is incomplete.");

FormDialogActivator::FormDialogActivator(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   {
      auto* widget = this->ui.navmeshGeneration;
      widget->clear();
      widget->addItem(tr("Collision"),    (uint)0);
      widget->addItem(tr("Bounding box"), (uint)loaded_form_type::form_flag::navmesh_generation_obb);
      widget->addItem(tr("Filter"),       (uint)loaded_form_type::form_flag::navmesh_generation_filter);
      widget->addItem(tr("Ground"),       (uint)loaded_form_type::form_flag::navmesh_generation_ground);
   }
   this->ui.soundActivate->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundLooping->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.waterType->setAllowedFormType(dovah::form_type::water_type);
   this->ui.defaultInteractKeyword->setAllowedFormType(dovah::form_type::keyword);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   this->load(); // this creates the working copy.
}
void FormDialogActivator::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   auto form_flags = this->stub->get_record_flags();
   
   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.soundActivate, working.activation_sound, working);
   ui::bind(this->ui.soundLooping,  working.looping_sound,    working);
   {
      if (form_flags & loaded_form_type::form_flag::navmesh_generation_filter) {
         this->ui.navmeshGeneration->setCurrentIndex(this->ui.navmeshGeneration->findData(loaded_form_type::form_flag::navmesh_generation_filter));
      } else if (form_flags & loaded_form_type::form_flag::navmesh_generation_ground) {
         this->ui.navmeshGeneration->setCurrentIndex(this->ui.navmeshGeneration->findData(loaded_form_type::form_flag::navmesh_generation_ground));
      } else if (form_flags & loaded_form_type::form_flag::navmesh_generation_obb) {
         this->ui.navmeshGeneration->setCurrentIndex(this->ui.navmeshGeneration->findData(loaded_form_type::form_flag::navmesh_generation_obb));
      } else {
         this->ui.navmeshGeneration->setCurrentIndex(this->ui.navmeshGeneration->findData(0));
      }
   }
   ui::bind(this->ui.waterType, working.water_type, working);
   ui::bind(this->ui.flagNoDisplacement, working.activator_flags, loaded_form_type::activator_flag::no_displacement);
   this->ui.name->setText(editor.convert_localized_string(working.name));
   //
   this->ui.activateTextOverride->setText(editor.convert_localized_string(working.activation_verb));
   {  // Flags
      ui::bind(this->ui.flagIgnoredBySandbox, working.activator_flags, loaded_form_type::activator_flag::ignored_by_sandbox);
      
      auto& record_flags = this->record_flags();
      ui::bind(this->ui.flagChildCanUse,     record_flags, loaded_form_type::form_flag::child_can_use);
      ui::bind(this->ui.flagDangerous,       record_flags, loaded_form_type::form_flag::dangerous);
      ui::bind(this->ui.flagHasTreeLOD,      record_flags, loaded_form_type::form_flag::has_tree_lod);
      ui::bind(this->ui.flagIgnoreObjectInteraction, record_flags, loaded_form_type::form_flag::ignore_object_interaction);
      ui::bind(this->ui.flagIsMarker,        record_flags, loaded_form_type::form_flag::is_marker);
      ui::bind(this->ui.flagMustUpdateAnims, record_flags, loaded_form_type::form_flag::must_update_anims);
      ui::bind(this->ui.flagObstacle,        record_flags, loaded_form_type::form_flag::obstacle);
      ui::bind(this->ui.flagOnLocalMap,      record_flags, loaded_form_type::form_flag::hide_from_local_map);
      ui::bind(this->ui.flagRandomAnimStart, record_flags, loaded_form_type::form_flag::random_anim_start);
   }
   ui::bind(this->ui.defaultPrimitiveColor,  working.marker_color);
   ui::bind(this->ui.defaultInteractKeyword, working.interact_keyword, working);
   for (auto& ref : working.keywords.forms) {
      this->ui.keywords->addStub(ref.get_form_stub());
   }

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogActivator::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.name->text());
   editor.assign_localized_string(working.activation_verb, this->ui.activateTextOverride->text());
   static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "Navmesh form flags");
   this->ui.keywords->commitStubs(working.keywords.forms, working);

   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);
   this->ui.scriptListPane->commit();

   // TODO: EVERYTHING THAT DOESN'T MODIFY THE WORKING COPY IN REAL-TIME
}