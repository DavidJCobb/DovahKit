#include "./class.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogClass::FormDialogClass(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.bleedoutDefault->setRange(0, 1);
   ui::set_unsigned_range<int32_t>(this->ui.voicePoints);

   this->ui_attr_weights = {
      this->ui.attrWeightH,
      this->ui.attrWeightM,
      this->ui.attrWeightS,
   };
   this->ui_skill_weights = {
      this->ui.skillWeightOneHanded,
      this->ui.skillWeightTwoHanded,
      this->ui.skillWeightArchery,
      this->ui.skillWeightBlock,
      this->ui.skillWeightSmithing,
      this->ui.skillWeightHeavyArmor,
      this->ui.skillWeightLightArmor,
      this->ui.skillWeightPickpocket,
      this->ui.skillWeightLockpicking,
      this->ui.skillWeightSneak,
      this->ui.skillWeightAlchemy,
      this->ui.skillWeightSpeech,
      this->ui.skillWeightAlteration,
      this->ui.skillWeightConjuration,
      this->ui.skillWeightDestruction,
      this->ui.skillWeightIllusion,
      this->ui.skillWeightRestoration,
      this->ui.skillWeightEnchanting,
   };

   for (auto* widget : this->ui_attr_weights)
      ui::set_range<uint8_t>(widget);
   for (auto* widget : this->ui_skill_weights)
      ui::set_range<uint8_t>(widget);

   {
      auto* widget = this->ui.trainingSkill;
      widget->clear();
      widget->addItem(tr("One-Handed"),  (int)dovah::skill::one_handed);
      widget->addItem(tr("Two-Handed"),  (int)dovah::skill::two_handed);
      widget->addItem(tr("Archery"),     (int)dovah::skill::archery);
      widget->addItem(tr("Block"),       (int)dovah::skill::block);
      widget->addItem(tr("Smithing"),    (int)dovah::skill::smithing);
      widget->addItem(tr("Heavy Armor"), (int)dovah::skill::heavy_armor);
      widget->addItem(tr("Light Armor"), (int)dovah::skill::light_armor);
      widget->addItem(tr("Pickpocket"),  (int)dovah::skill::pickpocket);
      widget->addItem(tr("Lockpicking"), (int)dovah::skill::lockpicking);
      widget->addItem(tr("Sneak"),       (int)dovah::skill::sneak);
      widget->addItem(tr("Alchemy"),     (int)dovah::skill::alchemy);
      widget->addItem(tr("Speech"),      (int)dovah::skill::speech);
      widget->addItem(tr("Alteration"),  (int)dovah::skill::alteration);
      widget->addItem(tr("Conjuration"), (int)dovah::skill::conjuration);
      widget->addItem(tr("Destruction"), (int)dovah::skill::destruction);
      widget->addItem(tr("Illusion"),    (int)dovah::skill::illusion);
      widget->addItem(tr("Restoration"), (int)dovah::skill::restoration);
      widget->addItem(tr("Enchanting"),  (int)dovah::skill::enchanting);
   }
   this->ui.trainingMaxLevel->setRange(1, std::numeric_limits<uint8_t>::max());

   this->load(); // this creates the working copy.
}
void FormDialogClass::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   this->ui.description->setPlainText(editor.convert_localized_string(working.description));
   ui::bind(this->ui.bleedoutDefault, working.bleedout_default);
   ui::bind(this->ui.voicePoints,     working.voice_points);
   for (size_t i = 0; i < dovah::skill_count; ++i) {
      ui::bind(this->ui_skill_weights[i], working.skill_weights[i]);
   }
   ui::bind(this->ui.attrWeightH, working.attribute_weights.health);
   ui::bind(this->ui.attrWeightM, working.attribute_weights.magicka);
   ui::bind(this->ui.attrWeightS, working.attribute_weights.stamina);

   QObject::connect(this->ui.trainingGroupbox, &QGroupBox::toggled, this, [this](bool checked) {
      auto& working = *this->form;
      if (!checked) {
         working.training.max_level = 0;
         return;
      }
      working.training.max_level = this->ui.trainingMaxLevel->value();
   });
   ui::bind(this->ui.trainingSkill, working.training.skill);
   QObject::connect(this->ui.trainingMaxLevel, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
      auto& working = *this->form;
      if (!this->ui.trainingGroupbox->isChecked())
         return;
      working.training.max_level = value;
   });

   ui::bind(this->ui.menuImage, working.icon);
   QObject::connect(this->ui.menuImage, &DKGameFilePicker::rawPathChanged, this, [this](QString path) {
      this->ui.menuImagePreview->setAsset(path);
   });
   this->ui.menuImagePreview->setAsset(QString::fromStdString(working.icon));
}
void FormDialogClass::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name,        this->ui.name->text());
   editor.assign_localized_string(working.description, this->ui.description->toPlainText());
}