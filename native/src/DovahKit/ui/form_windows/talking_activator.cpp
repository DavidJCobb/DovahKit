#include "./talking_activator.h"
#include "dovah/core.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"

FormDialogTalkingActivator::FormDialogTalkingActivator(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.loopSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.voicetype->setAllowedFormType(dovah::form_type::voicetype);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   this->load(); // this creates the working copy.
}
void FormDialogTalkingActivator::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.loopSound, working.looping_sound, working);
   ui::bind(this->ui.voicetype, working.voicetype,     working);
   {  // Flags
      {
         auto* src = this->ui.flagRadio;
         QObject::connect(src, &QCheckBox::toggled, this, [this](bool checked) {
            this->ui.flagNonPipBoy->setEnabled(checked);
            this->ui.flagContinuousBroadcast->setEnabled(checked);
         });
         bool checked = src->isChecked();
         this->ui.flagNonPipBoy->setEnabled(checked);
         this->ui.flagContinuousBroadcast->setEnabled(checked);
      }
      auto& record_flags = this->record_flags();
      ui::bind(this->ui.flagRadio,               record_flags, loaded_form_type::form_flag::radio_station);
      ui::bind(this->ui.flagNonPipBoy,           record_flags, loaded_form_type::form_flag::non_pip_boy);
      ui::bind(this->ui.flagNoVoiceFilter,       record_flags, loaded_form_type::form_flag::no_voice_filter);
      ui::bind(this->ui.flagContinuousBroadcast, record_flags, loaded_form_type::form_flag::continuous_broadcast);
      ui::bind(this->ui.flagRandomAnimStart,     record_flags, loaded_form_type::form_flag::random_anim_start);
      ui::bind_inverse(this->ui.flagOnLocalMap, record_flags, loaded_form_type::form_flag::hide_from_local_map);
   }
   for (auto& ref : working.keywords.forms) {
      this->ui.keywords->addStub(ref.get_form_stub());
   }

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogTalkingActivator::_save_impl() {
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
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.keywords->commitStubs(working.keywords.forms, working);

   this->ui.scriptListPane->commit();
}