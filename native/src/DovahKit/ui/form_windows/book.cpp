#include "./book.h"
#include "dovah/core.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/enum_dropdown_configs/skill.h"
#include "ui/utils/bind.h"

FormDialogBook::FormDialogBook(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.teachSpell->setAllowedFormType(dovah::form_type::spell);
   ui::enum_dropdown_configs::skill(this->ui.teachSkill, true, true);
   this->ui.menuDisplayObject->setAllowedFormType(dovah::form_type::statik);
   this->ui.takeSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.dropSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   this->load(); // this creates the working copy.
}
void FormDialogBook::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   #pragma region Left column
      ui::bind(this->ui.editorID, this->editor_id());
      this->ui.name->setText(editor.convert_localized_string(working.name));
      ui::bind(this->ui.weight, working.weight);
      ui::bind(this->ui.value,  working.value);
      {
         auto* radio_skill = this->ui.teachTypeSkill;
         auto* radio_spell = this->ui.teachTypeSpell;
         if (auto* use = std::get_if<dovah::form_reference_t>(&working.teaches)) {
            radio_spell->setChecked(true);
            this->ui.teachSpell->setFormStub(use->get_form_stub());
            this->ui.teachesStack->setCurrentWidget(this->ui.teachesStackSpell);
         } else {
            auto skill = std::get<int32_t>(working.teaches);
            radio_skill->setChecked(true);
            this->ui.teachSkill->setCurrentIndex(this->ui.teachSkill->findData((int)skill));
            this->ui.teachesStack->setCurrentWidget(this->ui.teachesStackSkill);
         }
         QObject::connect(radio_skill, &QRadioButton::toggled, this, [this](bool checked) {
            this->ui.teachesStack->setCurrentWidget(checked ? this->ui.teachesStackSkill : this->ui.teachesStackSpell);
         });
      }
      this->ui.model->initializeFrom(working.model);
      ui::bind(this->ui.menuDisplayObject, working.menu_display_object, working);
      this->ui.destructionData->initializeFrom(working.destruction_data);
      ui::bind(this->ui.flagCantBeTaken, working.flags, loaded_form_type::book_flag::cannot_be_taken);
      ui::bind(this->ui.takeSound, working.sounds.take, working);
      ui::bind(this->ui.dropSound, working.sounds.drop, working);
      ui::bind(this->ui.inventoryIcon, working.icons.inventory);
      ui::bind(this->ui.messageIcon, working.icons.message);
      this->ui.description->setPlainText(editor.convert_localized_string(working.description));
   #pragma endregion
   #pragma region Middle column
      this->ui.bookContent->setPlainText(editor.convert_localized_string(working.text));
   #pragma endregion
   #pragma region Right column
      this->ui.scriptListPane->setFormWorkingCopy(&working);
      for (auto& ref : working.keywords.forms) {
         this->ui.keywords->addStub(ref.get_form_stub());
      }
   #pragma endregion
}
void FormDialogBook::_save_impl() {
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
   {
      auto& dst_variant = working.teaches;
      if (this->ui.teachTypeSpell->isChecked()) {
         this->write_form_ref(dst_variant.emplace<dovah::form_reference_t>(), this->ui.teachSpell->formStub());
      } else {
         if (auto* use = std::get_if<dovah::form_reference_t>(&dst_variant)) {
            use->set(working, nullptr);
            dst_variant.emplace<int32_t>();
         }
         auto& dst_data = std::get<int32_t>(dst_variant);
         dst_data = this->ui.teachSkill->currentData().toInt();
      }
   }
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);
   editor.assign_localized_string(working.description, this->ui.description->toPlainText());

   editor.assign_localized_string(working.text, this->ui.bookContent->toPlainText());

   this->ui.scriptListPane->commit();
   this->ui.keywords->commitStubs(working.keywords.forms, working);
}