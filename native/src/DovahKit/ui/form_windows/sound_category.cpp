#include "./sound_category.h"
#include "dovah/core.h"
#include "editor/subsystems/per_form_windows/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

// "categorize" buttons
#include "widgets/DKFormPickerDialog.h"
#include "dovah/forms/SoundDescriptor.h"
#include "./sound_descriptor.h"

FormDialogSoundCategory::FormDialogSoundCategory(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);

   this->ui.parent->setAllowedFormType(dovah::form_type::sound_category);
   this->ui.parent->setCustomFilter(this->_filters.exclude_self);

   QObject::connect(this->ui.buttonCategorizeCategories, &QPushButton::clicked, this, [this]() {
      auto* dialog = new DKFormPickerDialog(this);
      dialog->setAllowedFormType(dovah::form_type::sound_category);
      dialog->setAllowMultiSelect(true);
      dialog->setCustomFilter(this->_filters.exclude_self);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      if (dialog->exec() == QDialog::Accepted) {
         auto& editor = DovahKitCore::get();
         auto& pfwins = dovahkit::subsystems::per_form_windows::core::get();

         auto sel = dialog->selectedFormStubs();
         for (dovah::form_stub* stub : sel) {
            if (auto* working = stub->working_copy) {
               ((loaded_form_type*)working)->parent.set(*working, this->formStub());
               pfwins.for_each_form_edit_dialog([stub](FormEditDialogInterface* intfc) {
                  if (intfc->formStub() != stub)
                     return true;

                  auto* dialog = dynamic_cast<FormDialogSoundCategory*>(intfc);
                  if (dialog) {
                     dialog->forceRefreshParentCategory();
                  }
                  return false;
               });
            }
            auto loaded = stub->load().ptr_cast<loaded_form_type>();
            if (loaded) {
               loaded->parent.set(*loaded, this->formStub());
               stub->set_edited(true);
               emit editor.formModified(stub);
            }
         }
      }
   });
   QObject::connect(this->ui.buttonCategorizeSounds, &QPushButton::clicked, this, [this]() {
      auto* dialog = new DKFormPickerDialog(this);
      dialog->setAllowedFormType(dovah::form_type::sound_descriptor);
      dialog->setAllowMultiSelect(true);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      if (dialog->exec() == QDialog::Accepted) {
         auto& editor = DovahKitCore::get();
         auto& pfwins = dovahkit::subsystems::per_form_windows::core::get();

         auto sel = dialog->selectedFormStubs();
         for (dovah::form_stub* stub : sel) {
            if (auto* working = stub->working_copy) {
               ((dovah::loaded_forms::SoundDescriptor*)working)->category.set(*working, this->formStub());
               pfwins.for_each_form_edit_dialog([stub](FormEditDialogInterface* intfc) {
                  if (intfc->formStub() != stub)
                     return true;

                  auto* dialog = dynamic_cast<FormDialogSoundDescriptor*>(intfc);
                  if (dialog) {
                     dialog->forceRefreshParentCategory();
                  }
                  return false;
               });
            }
            auto loaded = stub->load().ptr_cast<dovah::loaded_forms::SoundDescriptor>();
            if (loaded) {
               loaded->category.set(*loaded, this->formStub());
               stub->set_edited(true);
               emit editor.formModified(stub);
            }
         }
      }
   });

   this->load(); // this creates the working copy.
}

void FormDialogSoundCategory::forceRefreshParentCategory() {
   auto* widget  = this->ui.parent;
   auto  blocker = QSignalBlocker(widget);
   widget->setFormStub(this->form->parent.get_form_stub());
}

void FormDialogSoundCategory::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(this->formStub());

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.parent, working.parent, working);
   ui::bind(this->ui.flagMuteUnderwater, working.flags, loaded_form_type::sound_category_flag::mute_when_submerged);
   ui::bind(this->ui.flagShowInMenu, working.flags, loaded_form_type::sound_category_flag::show_in_audio_menu);
   {
      auto* widget = this->ui.staticVolumeMult;
      auto& value  = working.static_volume_mult;
      widget->setValue((double)value / 0xFFFF);
      QObject::connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [&value](double v) {
         value = v * 0xFFFF;
      });
   }
   {
      auto* widget = this->ui.defaultMenuValue;
      auto& value  = working.default_menu_value;
      widget->setValue((double)value / 0xFFFF);
      QObject::connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [&value](double v) {
         value = v * 0xFFFF;
      });
   }
}
void FormDialogSoundCategory::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   gls.assign_localized_string(working.name, this->ui.name->text());
}