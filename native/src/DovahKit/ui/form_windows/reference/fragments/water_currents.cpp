#include "./water_currents.h"
#include <QGroupBox>
#include <QDoubleSpinBox>
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/extra_data/types/w/water_current_zone_data.h"
#include "dovah/forms/components/extra_data/types/w/water_data.h"
#include "dovah/forms/ObjectReference.h"
#include "ui/utils/set_range.h"
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace ui::reference::fragments {
   void water_currents::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;

      for (auto* spinbox : this->controls.velocity.angular.all)
         spinbox->setRange(-360.0F, 360.0F);
      for (auto* spinbox : this->controls.velocity.linear.all)
         ui::set_range<float>(spinbox);
   }
   void water_currents::load(loaded_form_type& form) {
      this->stub = &form.stub;
      
      if (!can_have_currents(form)) {
         this->controls.groupbox->setEnabled(false);
         return;
      }

      cobb::vector3<float> vel_linear;
      cobb::vector3<float> vel_angular;

      auto* base_form = form.base_form.get_form_stub();
      if (base_form && base_form->formID == dovah::hardcoded_form_ids::WaterCurrentZoneMarker) {
         if (auto* extra = form.extra_data.get<extra_data_types::water_current_zone_data>()) {
            vel_linear  = extra->velocity.linear;
            vel_angular = extra->velocity.angular;
         }
      } else {
         if (auto* extra = form.extra_data.get<extra_data_types::water_data>()) {
            const auto size = extra->data.size();
            if (size >= 1) {
               vel_linear = extra->data[0].velocity;
               if (size >= 2) {
                  vel_angular = extra->data[1].velocity;
               }
            }
         }
      }

      for (size_t i = 0; i < 3; ++i) {
         this->controls.velocity.angular.all[i]->setValue(vel_angular[i]);
         this->controls.velocity.linear.all[i]->setValue(vel_linear[i]);
      }
   }
   void water_currents::save(loaded_form_type& form) {
      if (!can_have_currents(form))
         return;
      cobb::vector3<float> vel_linear;
      cobb::vector3<float> vel_angular;
      for (size_t i = 0; i < 3; ++i) {
         vel_linear[i]  = this->controls.velocity.linear.all[i]->value();
         vel_angular[i] = this->controls.velocity.angular.all[i]->value();
      }

      auto* base_form = form.base_form.get_form_stub();
      if (base_form && base_form->formID == dovah::hardcoded_form_ids::WaterCurrentZoneMarker) {
         auto* extra = form.extra_data.get_or_create<extra_data_types::water_current_zone_data>();
         extra->velocity.linear  = vel_linear;
         extra->velocity.angular = vel_angular;
      } else {
         auto* extra = form.extra_data.get_or_create<extra_data_types::water_data>();
         if (extra->data.size() < 2) {
            extra->data.resize(2);
         }
         extra->data[0].velocity = vel_linear;
         extra->data[1].velocity = vel_angular;
      }
   }
   bool water_currents::can_have_currents(loaded_form_type& form) const {
      dovah::form_stub* base_form = form.base_form.get_form_stub();
      if (!base_form)
         return false;
      return base_form->test_record_flags(1 << 19);
   }
}