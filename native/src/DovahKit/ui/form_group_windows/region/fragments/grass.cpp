#include "./grass.h"
#include <QCheckBox>
#include <QSpinBox>
#include "helpers/bound_mem_fn.h"
#include "../RegionsDialog.h"

namespace ui::region::fragments {
   grass::grass(RegionsDialog& o) : owner(o) {
   }
   void grass::set_controls(controls&& src) {
      this->ui = std::move(src);

      QObject::connect(this->ui.header.enable,  &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_header_edited));
      QObject::connect(this->ui.header.override, &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_header_edited));
      QObject::connect(this->ui.header.priority, qOverload<int>(&QSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_header_edited));
   }
   void grass::reload() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.grass;
      this->ui.header.enable->setEnabled(data.stub != nullptr);
      if (!data.stub || !opt_coll.has_value()) {
         this->ui.header.enable->setChecked(false);
         this->ui.header.override->setChecked(false);
         this->ui.header.override->setEnabled(false);
         this->ui.header.priority->setValue(0);
         this->ui.header.priority->setEnabled(false);
         return;
      }
      auto& src = opt_coll.value();
      this->ui.header.enable->setChecked(true);
      this->ui.header.override->setChecked(src.override);
      this->ui.header.override->setEnabled(true);
      this->ui.header.priority->setValue(src.priority);
      this->ui.header.priority->setEnabled(true);
   }
   void grass::commit() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.grass;
      if (this->ui.header.enable->isChecked()) {
         if (!opt_coll.has_value()) {
            opt_coll.emplace();
         }
         auto& dst = opt_coll.value();
         dst.override = this->ui.header.override->isChecked();
         dst.priority = this->ui.header.priority->value();
      } else {
         opt_coll.reset();
      }
   }

   void grass::on_header_edited() {
      bool enabled = this->ui.header.enable->isChecked();
      this->ui.header.override->setEnabled(enabled);
      this->ui.header.priority->setEnabled(enabled);
      this->owner.on_region_modified({});
   }
   void grass::on_data_edited() {
      this->owner.on_region_modified({});
   }
}