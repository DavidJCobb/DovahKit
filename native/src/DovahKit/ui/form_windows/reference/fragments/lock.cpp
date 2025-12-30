#include "./lock.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include "widgets/DKFormPicker.h"
#include "dovah/forms/components/extra_data/types/l/lock.h"
#include "dovah/forms/ObjectReference.h"

#include "dovah/forms/Door.h"

namespace ui::reference::fragments {
   void lock::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      
      controls.key->setAllowedFormType(dovah::form_type::key);
      {
         auto* widget = controls.level;
         widget->clear();
         widget->addItem(owner.tr("Novice",       "lock levels"),   1);
         widget->addItem(owner.tr("Apprentice",   "lock levels"),  25);
         widget->addItem(owner.tr("Adept",        "lock levels"),  50);
         widget->addItem(owner.tr("Expert",       "lock levels"),  75);
         widget->addItem(owner.tr("Master",       "lock levels"), 100);
         widget->addItem(owner.tr("Requires Key", "lock levels"), 255);
      }
   }
   void lock::load(loaded_form_type& form) {
      this->stub = &form.stub;

      if (!can_be_locked(form)) {
         this->controls.groupbox->setChecked(false);
         this->controls.groupbox->setEnabled(false);
         return;
      }
      this->controls.groupbox->setEnabled(true);
      if (auto* extra = form.extra_data.get<extra_data_type>()) {
         this->controls.groupbox->setChecked(true);
         this->controls.key->setFormStub(extra->key.get_form_stub());
         this->controls.level->setCurrentIndex(this->controls.level->findData(extra->level));
         this->controls.is_leveled->setChecked(extra->flags & extra_data_type::flag::leveled);
      } else {
         this->controls.groupbox->setChecked(false);
      }
   }
   void lock::save(loaded_form_type& form) {
      if (this->controls.groupbox->isChecked()) {
         form.extra_data.remove<extra_data_type>(form);
      } else {
         auto* extra = form.extra_data.get_or_create<extra_data_type>();
         extra->key.set(form, this->controls.key->formStub());
         extra->level = this->controls.level->currentData().toInt();
         cobb::edit_bit(extra->flags, extra_data_type::flag::leveled, this->controls.is_leveled->isChecked());
      }
   }

   bool lock::can_be_locked(const loaded_form_type& form) {
      auto* base_form = form.base_form.get_form_stub();
      if (!base_form)
         return false;
      switch (base_form->form_type) {
         case dovah::form_type::container:
            return true;
         case dovah::form_type::door:
            {
               auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Door>();
               if (!loaded)
                  break;
               if (loaded->door_flags & dovah::loaded_forms::Door::door_flag::automatic)
                  return false;
            }
            return true;
      }
      return false;
   }
}