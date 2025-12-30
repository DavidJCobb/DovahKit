#include "./light.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QVariant>
#include "widgets/DKFloatSlider.h"
#include "dovah/forms/components/extra_data/types/l/light.h"
#include "dovah/forms/components/extra_data/types/r/radius.h"
#include "dovah/forms/Light.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/utils/default_light_emitter_shadow_depth_bias.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace ui::reference::fragments {
   void light::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      
      controls.fov.spinbox->setRange(0, 179.9);
      ui::set_unsigned_range<float>(controls.fade.spinbox);
      ui::set_unsigned_range<float>(controls.end_cap.spinbox);
      controls.depth_bias.slider->setRange(0, 50);
      controls.depth_bias.spinbox->setRange(0, 50);

      QObject::connect(this->controls.depth_bias.slider, &DKFloatSlider::valueChanged, &owner, [this](float v) {
         this->controls.depth_bias.spinbox->setValue(v);
      });
      QObject::connect(this->controls.depth_bias.spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), &owner, [this](double v) {
         const auto blocker = QSignalBlocker(this->controls.depth_bias.slider);
         this->controls.depth_bias.slider->setValue(v);
      });
   }
   void light::load(loaded_form_type& form, uint32_t& record_flags) {
      this->stub = &form.stub;

      auto* base_form = form.base_form.get_form_stub();
      if (!base_form || base_form->form_type != dovah::form_type::light) {
         this->controls.groupbox->setEnabled(false);
         return;
      }
      this->controls.groupbox->setEnabled(true);
      
      if (auto* extra = form.extra_data.get<extra_data_type>()) {
         this->controls.groupbox->setChecked(true);
         this->controls.fov.spinbox->setValue(extra->fov);
         this->controls.fade.spinbox->setValue(extra->fade);
         this->controls.end_cap.spinbox->setValue(extra->end_distance_cap);
         this->controls.depth_bias.spinbox->setValue(extra->shadow_depth_bias);
      } else {
         auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Light>();
         if (loaded) {
            this->controls.fov.spinbox->setValue(loaded->fov);
            this->controls.fade.spinbox->setValue(loaded->fade);
            this->controls.end_cap.spinbox->setValue(loaded->radius); // TODO: is this correct?
         }
      }

      if (auto* extra = form.extra_data.get<extra_data_types::radius>())
         this->controls.radius.spinbox->setValue(extra->value);
      else {
         auto* base_form = this->loaded().base_form.get_form_stub();
         if (!base_form)
            return;
         auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Light>();
         if (loaded)
            this->controls.radius.spinbox->setValue(loaded->radius);
      }

      QObject::connect(this->controls.fov.reset, &QPushButton::clicked, [this, &form]() {
         auto* base_form = this->loaded().base_form.get_form_stub();
         if (!base_form)
            return;
         auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Light>();
         if (loaded)
            this->controls.fov.spinbox->setValue(loaded->fov);
      });
      QObject::connect(this->controls.fade.reset, &QPushButton::clicked, [this, &form]() {
         auto* base_form = this->loaded().base_form.get_form_stub();
         if (!base_form)
            return;
         auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Light>();
         if (loaded)
            this->controls.fade.spinbox->setValue(loaded->fade);
      });
      QObject::connect(this->controls.depth_bias.reset, &QPushButton::clicked, [this, &form]() {
         float bias = dovah::utils::default_light_emitter_shadow_depth_bias(form.stub, true);
         this->controls.depth_bias.spinbox->setValue(bias);
      });
      ui::bind(this->controls.flags.can_cast_shadows,     record_flags, loaded_form_type::form_flag::casts_shadows);
      ui::bind(this->controls.flags.does_not_light_land,  record_flags, loaded_form_type::form_flag::doesnt_light_landscape);
      ui::bind(this->controls.flags.does_not_light_water, record_flags, loaded_form_type::form_flag::doesnt_light_water);
      ui::bind(this->controls.flags.never_fades,          record_flags, loaded_form_type::form_flag::never_fades);
   }
   void light::save(loaded_form_type& form, uint32_t& record_flags) {
      auto* base_form = form.base_form.get_form_stub();
      if (!base_form || base_form->form_type != dovah::form_type::light) {
         return;
      }

      const float radius = this->controls.radius.spinbox->value();
      const float fov    = this->controls.fov.spinbox->value();
      const float fade   = this->controls.fade.spinbox->value();
      const float cap    = this->controls.end_cap.spinbox->value();
      const float bias   = this->controls.depth_bias.spinbox->value();
      
      const bool any_changed = [&]() {
         auto* base_form = this->loaded().base_form.get_form_stub();
         if (!base_form)
            return true;
         auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Light>();
         if (!loaded)
            return true;
         return (
            loaded->radius != radius
         || loaded->fade   != fade
         || loaded->fov    != fov
         );
      }();

      if (any_changed) {
         auto* extra = form.extra_data.get_or_create<extra_data_types::light>();
         extra->fade = fade;
         extra->fov  = fov;
         extra->end_distance_cap  = cap;
         extra->shadow_depth_bias = bias;
         form.extra_data.get_or_create<extra_data_types::radius>()->value = radius;
      } else {
         form.extra_data.remove<extra_data_types::light>(form);
         form.extra_data.remove<extra_data_types::radius>(form);
      }
   }

   light::loaded_form_type& light::loaded() {
      assert(this->stub != nullptr);
      return *this->stub->get_working_copy<loaded_form_type>();
   }
}