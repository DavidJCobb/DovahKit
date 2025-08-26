#include "./FormSubdialogPackageLocation.h"
#include "dovah/forms/structs/custom_packages/package_data/_target_base.h"
#include "dovah/forms/structs/typed_package_info/custom.h"
#include "dovah/forms/Package.h"
#include "dovah/forms/Quest.h"
#include "dovah/form_stub.h"
#include "editor/localize/package_interrupt_override_target.h"
#include "ui/types/packages/package_data_declaration.h"
#include "ui/types/packages/package_location.h"
#include "../shared/InteriorCellPickerFilter.h"

FormSubdialogPackageLocation::FormSubdialogPackageLocation(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->setInterruptOverrideType(dovah::packages::interrupt_override_type::none);
   this->setOwningQuest(nullptr);

   this->ui.interiorCell->setAllowedFormType(dovah::form_type::cell);
   {
      auto* filter = new InteriorCellPickerFilter(this);
      this->ui.interiorCell->setCustomFilter(filter);
   }
   this->ui.linkedRefKeyword->setAllowedFormType(dovah::form_type::keyword);

   for (auto& pair : std::array{
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypeLinkedRef,       this->ui.linkedRefKeyword },
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypeInteriorCell,    this->ui.interiorCell },
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypeRef,             this->ui.ref },
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypeLocAlias,        this->ui.locAlias },
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypeRefAlias,        this->ui.refAlias },
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypeInterruptTarget, this->ui.interruptTarget },
      std::pair<QRadioButton*, QWidget*>{ this->ui.locationTypePackdataTarget,  this->ui.packdataTarget },
   }) {
      QObject::connect(pair.first, &QRadioButton::toggled, pair.second, &QWidget::setEnabled);
   }

   this->ui.locationTypeAtPackLoc->setChecked(true);
}

void FormSubdialogPackageLocation::setInterruptOverrideType(dovah::packages::interrupt_override_type t) {
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
   this->ui.locationTypeInterruptTarget->setEnabled(!empty);
   this->ui.interruptTarget->setEnabled(!empty);
}
void FormSubdialogPackageLocation::setOwningPackage(dovah::form_stub* stub) {
   this->ui.packdataTarget->clear();
   this->ui.locationTypePackdataTarget->setEnabled(false);
   this->ui.packdataTarget->setEnabled(false);
   if (!stub)
      return;
   if (stub->form_type != dovah::form_type::package)
      return;
   auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Package>();
   if (!loaded)
      return;
   auto* custom = dynamic_cast<dovah::loaded_forms::structs::typed_package_info::custom*>(loaded->typed_info);
   if (!custom)
      return;
   
   std::vector<uint8_t> ids;
   for (auto& pair : custom->data.values.entries) {
      if (pair.unique_id == ui::types::packages::package_data_declaration::no_unique_id)
         continue;
      if (!pair.value)
         continue;
      if (auto* casted = dynamic_cast<dovah::loaded_forms::structs::custom_packages::package_data_target_base*>(pair.value.get())) {
         ids.push_back(pair.unique_id);
      }
   }
   if (ids.empty())
      return;
   bool any = false;
   for (auto id : ids) {
      for (auto& decl : custom->data.declarations.entries) {
         if (decl.unique_id == id) {
            any = true;
            this->ui.packdataTarget->addItem(QString::fromStdString(decl.name), (int)id);
         }
      }
   }
   if (any) {
      this->ui.locationTypePackdataTarget->setEnabled(true);
      this->ui.packdataTarget->setEnabled(true);
   }
}
void FormSubdialogPackageLocation::setOwningQuest(dovah::form_stub* stub) {
   this->ui.locAlias->clear();
   this->ui.refAlias->clear();
   this->ui.locationTypeLocAlias->setEnabled(false);
   this->ui.locationTypeRefAlias->setEnabled(false);
   this->ui.locAlias->setEnabled(false);
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
         case dovah::loaded_forms::Alias::alias_type::location:
            target = this->ui.locAlias;
            this->ui.locAlias->setEnabled(true);
            this->ui.locationTypeLocAlias->setEnabled(true);
            break;
         case dovah::loaded_forms::Alias::alias_type::reference:
            target = this->ui.refAlias;
            this->ui.refAlias->setEnabled(true);
            this->ui.locationTypeRefAlias->setEnabled(true);
            break;
         default:
            continue;
      }
      target->addItem(QString::fromStdString(alias->name), alias->id);
   }
}

FormSubdialogPackageLocation::value_type FormSubdialogPackageLocation::value() const {
   value_type dst;
   if (this->ui.locationTypeAtPackLoc->isChecked()) {
      dst.set_type(dovah::packages::location_type::at_package_location);
   } else if (this->ui.locationTypeEditorLoc->isChecked()) {
      dst.set_type(dovah::packages::location_type::near_editor_location);
   } else if (this->ui.locationTypeInteriorCell->isChecked()) {
      dst.set_type(dovah::packages::location_type::interior_cell);
      *dst.as_type<dovah::packages::location_type::interior_cell>() = this->ui.interiorCell->formStub();
   } else if (this->ui.locationTypeInterruptTarget->isChecked()) {
      dst.set_type(dovah::packages::location_type::interrupt_override_target);
      *dst.as_type<dovah::packages::location_type::interrupt_override_target>() = (dovah::packages::interrupt_override_target)this->ui.interruptTarget->currentData().toInt();
   } else if (this->ui.locationTypeLinkedRef->isChecked()) {
      dst.set_type(dovah::packages::location_type::linked_ref);
      *dst.as_type<dovah::packages::location_type::linked_ref>() = this->ui.linkedRefKeyword->formStub();
   } else if (this->ui.locationTypeLocAlias->isChecked()) {
      dst.set_type(dovah::packages::location_type::location_alias);
      *dst.as_type<dovah::packages::location_type::location_alias>() = this->ui.locAlias->currentData().toInt();
   } else if (this->ui.locationTypePackdataTarget->isChecked()) {
      dst.set_type(dovah::packages::location_type::package_data_target);
      *dst.as_type<dovah::packages::location_type::package_data_target>() = this->ui.packdataTarget->currentData().toInt();
   } else if (this->ui.locationTypePackStartLoc->isChecked()) {
      dst.set_type(dovah::packages::location_type::near_package_start_location);
   } else if (this->ui.locationTypeRef->isChecked()) {
      dst.set_type(dovah::packages::location_type::reference);
      *dst.as_type<dovah::packages::location_type::reference>() = this->ui.ref->ref();
   } else if (this->ui.locationTypeRefAlias->isChecked()) {
      dst.set_type(dovah::packages::location_type::reference_alias);
      *dst.as_type<dovah::packages::location_type::reference_alias>() = this->ui.refAlias->currentData().toInt();
   } else if (this->ui.locationTypeSelf->isChecked()) {
      dst.set_type(dovah::packages::location_type::self);
   }
   return dst;
}
void FormSubdialogPackageLocation::setValue(const value_type& src) {
   switch (src.get_type()) {
      case dovah::packages::location_type::at_package_location:
      default:
         this->ui.locationTypeAtPackLoc->setChecked(true);
         break;
      case dovah::packages::location_type::near_editor_location:
         this->ui.locationTypeEditorLoc->setChecked(true);
         break;
      case dovah::packages::location_type::near_package_start_location:
         this->ui.locationTypePackStartLoc->setChecked(true);
         break;
      case dovah::packages::location_type::self:
         this->ui.locationTypeSelf->setChecked(true);
         break;

      case dovah::packages::location_type::interior_cell:
         this->ui.locationTypeInteriorCell->setChecked(true);
         this->ui.interiorCell->setFormStub(*src.as_type<dovah::packages::location_type::interior_cell>());
         break;
      case dovah::packages::location_type::interrupt_override_target:
         this->ui.locationTypeInterruptTarget->setChecked(true);
         {
            auto* widget = this->ui.interruptTarget;
            auto  value  = *src.as_type<dovah::packages::location_type::interrupt_override_target>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
      case dovah::packages::location_type::linked_ref:
         this->ui.locationTypeLinkedRef->setChecked(true);
         this->ui.linkedRefKeyword->setFormStub(*src.as_type<dovah::packages::location_type::linked_ref>());
         break;
      case dovah::packages::location_type::location_alias:
         this->ui.locationTypeLocAlias->setChecked(true);
         {
            auto* widget = this->ui.locAlias;
            auto  value  = *src.as_type<dovah::packages::location_type::location_alias>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
      case dovah::packages::location_type::package_data_target:
         this->ui.locationTypePackdataTarget->setChecked(true);
         {
            auto* widget = this->ui.packdataTarget;
            auto  value  = *src.as_type<dovah::packages::location_type::package_data_target>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
      case dovah::packages::location_type::reference:
         this->ui.locationTypeRef->setChecked(true);
         this->ui.ref->setRef(*src.as_type<dovah::packages::location_type::reference>());
         break;
      case dovah::packages::location_type::reference_alias:
         this->ui.locationTypeRefAlias->setChecked(true);
         {
            auto* widget = this->ui.refAlias;
            auto  value  = *src.as_type<dovah::packages::location_type::reference_alias>();
            auto  i = widget->findData((int)value);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         break;
   }
}