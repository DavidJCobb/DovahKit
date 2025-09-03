#include "./actor_value_info.h"
#include "dovah/core.h"
#include "dovah/data/actor_values.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "./actor_value_info/FormSubdialogActorValueInfoPerkTree.h"

FormDialogActorValueInfo::FormDialogActorValueInfo(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::item_indices_to_data(this->ui.skillCategory);
   ui::bind(this->ui.image, this->ui.texture);

   this->load(); // this creates the working copy.
}
void FormDialogActorValueInfo::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   bool is_skill = false;

   const dovah::actor_value_info* info = nullptr;
   for (const auto& item : dovah::all_actor_value_info) {
      if (item.formID == this->formStub()->formID) {
         info = &item;
         break;
      }
   }
   if (info) {
      is_skill = info->type == dovah::actor_value_type::skill;

      this->ui.hardcodedName->setText(QString::fromLatin1(info->name.data(), info->name.size()));
      switch (info->type) {
         case dovah::actor_value_type::ai_temperament:
            this->ui.hardcodedType->setText(tr("AI Temperament"));
            break;
         case dovah::actor_value_type::attribute:
            this->ui.hardcodedType->setText(tr("Attribute"));
            break;
         case dovah::actor_value_type::limb_condition:
            this->ui.hardcodedType->setText(tr("Limb Condition"));
            break;
         case dovah::actor_value_type::miscellaneous:
            this->ui.hardcodedType->setText(tr("Miscellaneous"));
            break;
         case dovah::actor_value_type::resistance:
            this->ui.hardcodedType->setText(tr("Resistance"));
            break;
         case dovah::actor_value_type::skill:
            this->ui.hardcodedType->setText(tr("Skill"));
            break;
         case dovah::actor_value_type::status:
            this->ui.hardcodedType->setText(tr("Status"));
            break;
      }
   }
   this->ui.buttonPerkTree->setEnabled(is_skill);
   this->ui.skillCategory->setEnabled(is_skill);
   this->ui.skillUseGroupbox->setEnabled(is_skill);
   this->ui.skillImproveGroupbox->setEnabled(is_skill);

   this->ui.name->setText(editor.convert_localized_string(working.name));
   this->ui.abbreviation->setText(QString::fromStdString(working.abbreviation));
   ui::bind(this->ui.skillCategory, working.skill_info.category);
   ui::bind(this->ui.skillUseMult, working.skill_info.skill_use_mult);
   ui::bind(this->ui.skillUseOffset, working.skill_info.skill_use_offset);
   ui::bind(this->ui.skillImproveMult, working.skill_info.skill_improve_mult);
   ui::bind(this->ui.skillImproveOffset, working.skill_info.skill_improve_offset);
   this->ui.description->setPlainText(editor.convert_localized_string(working.description));
   ui::bind(this->ui.image, working.icon);

   QObject::connect(this->ui.buttonPerkTree, &QPushButton::clicked, this, [this]() {
      FormSubdialogActorValueInfoPerkTree dialog;
      dialog.setOwningActorValue(this->formStub());
      dialog.exec();
   });
}
void FormDialogActorValueInfo::_save_impl() {
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
   working.abbreviation = this->ui.abbreviation->text().toStdString();
   editor.assign_localized_string(working.description, this->ui.description->toPlainText());
}