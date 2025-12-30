#include "./water_lights.h"
#include <QGroupBox>
#include <QTableView>
#include <QVariant>
#include "dovah/forms/ObjectReference.h"
#include "ui/utils/typical_tableview_config.h"
#include "../ObjectReferenceWaterLightsModel.h"
#include "./reflected_refs.h" // reflected_refs::is_placed_water

namespace ui::reference::fragments {
   void water_lights::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      
      this->model = new model_type(controls.view);
      controls.view->setModel(this->model);
      ui::typical_tableview_config(controls.view);

      static_assert(false, "TODO: It needs to be possible to add/remove entries via the context menu");
   }
   void water_lights::load(loaded_form_type& form) {
      this->stub = &form.stub;

      this->model->setSubject(&form.stub);
      if (!reflected_refs::is_placed_water(form))
         this->controls.groupbox->setEnabled(false);
   }
   void water_lights::save(loaded_form_type& form) {
   }
}