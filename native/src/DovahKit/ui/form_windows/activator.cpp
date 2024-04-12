#include "./activator.h"
#include "./_base_cpp.h"
#include "dovah/core.h"
#include "dovah/form_stub_addenda.h"
#include "helpers/qt/basic_bindings.h"
#include "widgets/DKFormNIFPicker.h"

#include "../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "The form-editing dialog for Activators is incomplete.");

FormDialogActivator::FormDialogActivator(dovah::form_stub* stub, QWidget* parent) : FormWorkingCopyEditDialogBase(dovah::form_type::activator, stub, parent) {
   form_dialog_helpers::initialize(*this, stub);
   //
   {
      auto* widget = this->ui.navmeshGeneration;
      widget->clear();
      widget->addItem(tr("Collision"),    (uint)0);
      widget->addItem(tr("Bounding box"), (uint)loaded_form_type::form_flag::navmesh_generation_obb);
      widget->addItem(tr("Filter"),       (uint)loaded_form_type::form_flag::navmesh_generation_filter);
      widget->addItem(tr("Ground"),       (uint)loaded_form_type::form_flag::navmesh_generation_ground);
   }
   this->ui.waterType->setAllowedFormType(dovah::form_type::water_type);
   this->ui.defaultInteractKeyword->setAllowedFormType(dovah::form_type::keyword);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "Constrain activation and looping sound form-types");

   this->load(); // this creates the working copy.
   //
   QObject::connect(this->ui.defaultPrimitiveColor, &DKColorPickerButton::colorChanged, this, [this](QColor value) {
      auto& working = *this->get_working_copy<loaded_form_type>();
      working.marker_color.r = value.red();
      working.marker_color.g = value.green();
      working.marker_color.b = value.blue();
   });
}
void FormDialogActivator::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->get_working_copy<loaded_form_type>();

   auto form_flags = this->stub->get_record_flags();
   
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   this->ui.soundActivate->setFormStub(working.activation_sound.get_form_stub());
   this->ui.soundLooping->setFormStub(working.looping_sound.get_form_stub());
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
   this->ui.waterType->setFormStub(working.water_type.get_form_stub());
   cobb::qt::bind(this->ui.flagNoDisplacement, working.activator_flags, loaded_form_type::activator_flag::no_displacement);
   this->ui.name->setText(editor.convert_localized_string(working.name));
   //
   this->ui.activateTextOverride->setText(editor.convert_localized_string(working.activation_verb));
   {  // Flags
      //
      // Only bind flags that aren't form flags. Form flags are handled in `_save_impl` since 
      // they're stored on the stub itself, not the working copy of the full form data.
      //
      cobb::qt::bind(this->ui.flagIgnoredBySandbox, working.activator_flags, loaded_form_type::activator_flag::ignored_by_sandbox);
      //
      this->ui.flagChildCanUse->setChecked(form_flags & loaded_form_type::form_flag::child_can_use);
      this->ui.flagDangerous->setChecked(form_flags & loaded_form_type::form_flag::dangerous);
      this->ui.flagHasTreeLOD->setChecked(form_flags & loaded_form_type::form_flag::has_tree_lod);
      this->ui.flagIgnoreObjectInteraction->setChecked(form_flags & loaded_form_type::form_flag::ignore_object_interaction);
      this->ui.flagIsMarker->setChecked(form_flags & loaded_form_type::form_flag::is_marker);
      this->ui.flagMustUpdateAnims->setChecked(form_flags & loaded_form_type::form_flag::must_update_anims);
      this->ui.flagObstacle->setChecked(form_flags & loaded_form_type::form_flag::obstacle);
      this->ui.flagOnLocalMap->setChecked(!(form_flags & loaded_form_type::form_flag::hide_from_local_map));
      this->ui.flagRandomAnimStart->setChecked(form_flags & loaded_form_type::form_flag::random_anim_start);
   }
   this->ui.defaultPrimitiveColor->setColor(QColor::fromRgb(
      working.marker_color.r,
      working.marker_color.g,
      working.marker_color.b
   ));
   this->ui.defaultInteractKeyword->setFormStub(working.interact_keyword.get_form_stub());
   for (auto& ref : working.keywords.forms) {
      this->ui.keywords->addStub(ref.get_form_stub());
   }

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogActivator::_save_impl() {
   //
   // FormDialogWorkingCopyBase will handle the task of saving the working copy. 
   // We just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->get_working_copy<loaded_form_type>();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   editor.assign_localized_string(working.name, this->ui.name->text());
   editor.assign_localized_string(working.activation_verb, this->ui.activateTextOverride->text());
   working.activation_sound.set(working, this->ui.soundActivate->formStub());
   working.looping_sound.set(working, this->ui.soundLooping->formStub());
   static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "Navmesh form flags");
   static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "Water type");
   {  // Form flags
      this->stub->edit_record_flags(loaded_form_type::form_flag::child_can_use, this->ui.flagChildCanUse->isChecked());
      this->stub->edit_record_flags(loaded_form_type::form_flag::dangerous, this->ui.flagDangerous->isChecked());
      this->stub->edit_record_flags(loaded_form_type::form_flag::has_tree_lod, this->ui.flagHasTreeLOD->isChecked());
      // "Ignored by Sandbox" is in a different flags mask.
      this->stub->edit_record_flags(loaded_form_type::form_flag::ignore_object_interaction, this->ui.flagIgnoreObjectInteraction->isChecked());
      this->stub->edit_record_flags(loaded_form_type::form_flag::is_marker, this->ui.flagIsMarker->isChecked());
      this->stub->edit_record_flags(loaded_form_type::form_flag::must_update_anims, this->ui.flagMustUpdateAnims->isChecked());
      // "No Displacement" is in a different flags mask.
      this->stub->edit_record_flags(loaded_form_type::form_flag::obstacle, this->ui.flagObstacle->isChecked());
      this->stub->edit_record_flags(loaded_form_type::form_flag::hide_from_local_map, !this->ui.flagOnLocalMap->isChecked());
      this->stub->edit_record_flags(loaded_form_type::form_flag::random_anim_start, this->ui.flagRandomAnimStart->isChecked());
   }
   static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "Primitive color");
   working.interact_keyword.set(working, this->ui.defaultInteractKeyword->formStub());
   this->ui.keywords->commitStubs(working.keywords.forms, working);

   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);
   this->ui.scriptListPane->commit();

   // TODO: EVERYTHING THAT DOESN'T MODIFY THE WORKING COPY IN REAL-TIME
}