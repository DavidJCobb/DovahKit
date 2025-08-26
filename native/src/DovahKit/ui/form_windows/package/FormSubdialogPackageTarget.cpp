#include "./FormSubdialogPackageTarget.h"
#include "dovah/forms/structs/custom_packages/package_data/_target_base.h"
#include "dovah/forms/structs/typed_package_info/custom.h"
#include "dovah/forms/Quest.h"
#include "dovah/form_stub.h"
#include "editor/localize/package_interrupt_override_target.h"
#include "editor/localize/package_object_type.h"
#include "ui/types/packages/package_data_declaration.h"
#include "ui/types/packages/package_target.h"

FormSubdialogPackageTarget::FormSubdialogPackageTarget(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->setInterruptOverrideType(dovah::packages::interrupt_override_type::none);
   this->setOwningQuest(nullptr);

   this->ui.linkedRefKeyword->setAllowedFormType(dovah::form_type::keyword);
   this->ui.baseForm->setAllowedFormTypes({
      dovah::form_type::activator,
      dovah::form_type::actor_base,
      dovah::form_type::ammo,
      dovah::form_type::armor,
      dovah::form_type::book,
      dovah::form_type::container,
      dovah::form_type::door,
      dovah::form_type::faction, // per xEdit. strange.
      dovah::form_type::formlist,
      dovah::form_type::furniture,
      dovah::form_type::idle_marker,
      dovah::form_type::key,
      dovah::form_type::light,
      dovah::form_type::misc_item,
      dovah::form_type::movable_static,
      dovah::form_type::potion,
      dovah::form_type::scroll,
      dovah::form_type::shout, // per xEdit. strange.
      dovah::form_type::spell, // per xEdit. strange.
      dovah::form_type::statik,
      dovah::form_type::weapon
   });

   for (auto& pair : std::array{
      std::pair<QRadioButton*, QWidget*>{ this->ui.targetTypeLinkedRef,       this->ui.linkedRefKeyword },
      std::pair<QRadioButton*, QWidget*>{ this->ui.targetTypeRef,             this->ui.ref },
      std::pair<QRadioButton*, QWidget*>{ this->ui.targetTypeObject,          this->ui.baseForm },
      std::pair<QRadioButton*, QWidget*>{ this->ui.targetTypeObjectType,      this->ui.objectType },
      std::pair<QRadioButton*, QWidget*>{ this->ui.targetTypeRefAlias,        this->ui.refAlias },
      std::pair<QRadioButton*, QWidget*>{ this->ui.targetTypeInterruptTarget, this->ui.interruptTarget },
   }) {
      QObject::connect(pair.first, &QRadioButton::toggled, pair.second, &QWidget::setEnabled);
   }

   {
      auto* widget = this->ui.objectType;
      widget->clear();
      using enum dovah::packages::object_type;
      for (auto v : std::array{
         none,
         activators,
         armor,
         books,
         clothing,
         containers,
         doors,
         ingredients,
         lights,
         misc,
         flora,
         furniture,
         weapons_any,
         ammo,
         actors_characters, // NPCs. Pre-Skyrim, Actor was the base class of Character and Creature; Skyrim merged them.
         actors_creatures,  // Creatures.
         keys,
         alchemy,
         food,
         all_combat_wearable,
         all_wearable,
         weapons_ranged,
         weapons_melee,
         weapons_none,
         actor_effects_any,
         actor_effects_range_target,
         actor_effects_range_touch,
         actor_effects_range_self,
         actors_any,
      }) {
         widget->addItem(editor::localize::package_object_type(v), (int)v);
      }
   }
}

void FormSubdialogPackageTarget::setInterruptOverrideType(dovah::packages::interrupt_override_type t) {
   this->ui.interruptTarget->clear();

   bool empty = true;
   for (auto target : std::array{
      dovah::packages::interrupt_override_target::threat_to_spectate,
      dovah::packages::interrupt_override_target::corpse_to_observe,
      dovah::packages::interrupt_override_target::ref_to_guard,
      dovah::packages::interrupt_override_target::trespasser,
      dovah::packages::interrupt_override_target::combat_target,
   }) {
      if (interrupt_override_for_target(target) != t)
         continue;
      empty = false;
      this->ui.interruptTarget->addItem(
         editor::localize::package_interrupt_override_target(target),
         (int)target
      );
   }
   this->ui.targetTypeInterruptTarget->setEnabled(!empty);
   this->ui.interruptTarget->setEnabled(!empty);
}
void FormSubdialogPackageTarget::setOwningQuest(dovah::form_stub* stub) {
   this->ui.refAlias->clear();
   this->ui.targetTypeRefAlias->setEnabled(false);
   this->ui.refAlias->setEnabled(false);
   if (!stub)
      return;
   if (stub->form_type != dovah::form_type::quest)
      return;
   auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Quest>();
   if (!loaded)
      return;
   
   for (auto* alias : loaded->aliases) {
      QComboBox* target = nullptr;
      switch (alias->type) {
         case dovah::loaded_forms::Alias::alias_type::reference:
            target = this->ui.refAlias;
            this->ui.refAlias->setEnabled(true);
            this->ui.targetTypeRefAlias->setEnabled(true);
            break;
         default:
            continue;
      }
      target->addItem(QString::fromStdString(alias->name), alias->id);
   }
}

FormSubdialogPackageTarget::value_type FormSubdialogPackageTarget::value() const {
   value_type dst;
   if (this->ui.targetTypeSelf->isChecked()) {
      dst.set_type(dovah::packages::target_type::self);
   } else if (this->ui.targetTypeLinkedRef->isChecked()) {
      dst.set_type(dovah::packages::target_type::linked_ref);
      *dst.as_type<dovah::packages::target_type::linked_ref>() = this->ui.linkedRefKeyword->formStub();
   } else if (this->ui.targetTypeRef->isChecked()) {
      dst.set_type(dovah::packages::target_type::reference);
      *dst.as_type<dovah::packages::target_type::reference>() = this->ui.ref->ref();
   } else if (this->ui.targetTypeAny->isChecked()) {
      dst.set_type(dovah::packages::target_type::object);
      *dst.as_type<dovah::packages::target_type::object>() = nullptr;
   } else if (this->ui.targetTypeObject->isChecked()) {
      dst.set_type(dovah::packages::target_type::object);
      *dst.as_type<dovah::packages::target_type::object>() = this->ui.baseForm->formStub();
   } else if (this->ui.targetTypeObjectType->isChecked()) {
      dst.set_type(dovah::packages::target_type::object_type);
      *dst.as_type<dovah::packages::target_type::object_type>() = (dovah::packages::object_type)this->ui.objectType->currentData().toInt();
   } else if (this->ui.targetTypeRefAlias->isChecked()) {
      dst.set_type(dovah::packages::target_type::reference_alias);
      *dst.as_type<dovah::packages::target_type::reference_alias>() = this->ui.refAlias->currentData().toInt();
   } else if (this->ui.targetTypeInterruptTarget->isChecked()) {
      dst.set_type(dovah::packages::target_type::interrupt_override_target);
      *dst.as_type<dovah::packages::target_type::interrupt_override_target>() = (dovah::packages::interrupt_override_target)this->ui.interruptTarget->currentData().toInt();
   }
   return dst;
}
void FormSubdialogPackageTarget::setValue(const value_type& src) {
   switch (src.get_type()) {
      case dovah::packages::target_type::self:
      default:
         this->ui.targetTypeSelf->setChecked(true);
         break;
      case dovah::packages::target_type::linked_ref:
         this->ui.targetTypeLinkedRef->setChecked(true);
         this->ui.linkedRefKeyword->setFormStub(*src.as_type<dovah::packages::target_type::linked_ref>());
         break;
      case dovah::packages::target_type::reference:
         this->ui.targetTypeRef->setChecked(true);
         this->ui.ref->setRef(*src.as_type<dovah::packages::target_type::reference>());
         break;
      case dovah::packages::target_type::object:
         {
            dovah::form_stub* form = *src.as_type<dovah::packages::target_type::object>();
            if (!form) {
               this->ui.targetTypeAny->setChecked(true);
            } else {
               this->ui.targetTypeObject->setChecked(true);
               this->ui.baseForm->setFormStub(form);
            }
         }
         break;
      case dovah::packages::target_type::object_type:
         this->ui.targetTypeObjectType->setChecked(true);
         {
            auto* widget = this->ui.objectType;
            auto  value  = *src.as_type<dovah::packages::target_type::object_type>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
      case dovah::packages::target_type::reference_alias:
         this->ui.targetTypeRefAlias->setChecked(true);
         {
            auto* widget = this->ui.refAlias;
            auto  value  = *src.as_type<dovah::packages::target_type::reference_alias>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
      case dovah::packages::target_type::interrupt_override_target:
         this->ui.targetTypeInterruptTarget->setChecked(true);
         {
            auto* widget = this->ui.interruptTarget;
            auto  value  = *src.as_type<dovah::packages::target_type::interrupt_override_target>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
   }
}