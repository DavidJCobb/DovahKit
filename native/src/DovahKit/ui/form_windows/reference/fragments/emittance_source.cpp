#include "./emittance_source.h"
#include <array>
#include <QRadioButton>
#include <QVariant>
#include "widgets/DKFormPicker.h"
#include "dovah/forms/components/extra_data/types/e/emittance_source.h"
#include "dovah/forms/ObjectReference.h"

namespace ui::reference::fragments {
   void emittance_source::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      
      controls.light.form->setAllowedFormType(dovah::form_type::light);
      controls.region.form->setAllowedFormType(dovah::form_type::region);

      this->controls.none.radio->setProperty("emittance-type", (int)emittance_type::none);
      this->controls.light.radio->setProperty("emittance-type", (int)emittance_type::internal);
      this->controls.region.radio->setProperty("emittance-type", (int)emittance_type::external);
      for (auto* radio : std::array{
         this->controls.none.radio,
         this->controls.light.radio,
         this->controls.region.radio,
      }) {
         QObject::connect(radio, &QAbstractButton::toggled, &owner, [this, radio](bool checked) {
            if (checked) {
               const auto type = (emittance_type)radio->property("emittance-type").toInt();
               this->set_type(type);
            }
         });
      }
   }
   void emittance_source::load(loaded_form_type& form) {
      this->stub = &form.stub;

      if (auto* extra = form.extra_data.get<extra_data_type>()) {
         auto* form = extra->form.get_form_stub();
         if (form && form->form_type != dovah::form_type::light && form->form_type != dovah::form_type::region)
            form = nullptr;

         if (form) {
            if (form->form_type == dovah::form_type::light) {
               this->set_type(emittance_type::internal);
               this->controls.light.form->setFormStub(form);
            } else {
               this->set_type(emittance_type::external);
               this->controls.region.form->setFormStub(form);
            }
         } else {
            this->set_type(emittance_type::none);
         }
      } else {
         this->set_type(emittance_type::none);
      }
   }
   void emittance_source::save(loaded_form_type& form) {
      dovah::form_stub* source = nullptr;
      if (this->controls.light.radio->isChecked()) {
         source = this->controls.light.form->formStub();
      } else if (this->controls.region.radio->isChecked()) {
         source = this->controls.region.form->formStub();
      }
      if (source) {
         form.extra_data.get_or_create<extra_data_type>()->form.set(form, source);
      } else {
         form.extra_data.remove<extra_data_type>(form);
      }
   }
   void emittance_source::set_type(emittance_type t) {
      this->controls.light.form->setEnabled(t == emittance_type::internal);
      this->controls.region.form->setEnabled(t == emittance_type::external);
   }
}