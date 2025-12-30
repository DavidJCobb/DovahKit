#include "./reflected_refs.h"
#include <QGroupBox>
#include <QTableView>
#include <QVariant>
#include "dovah/form_stubs/helpers/get_activator_water_type.h"
#include "dovah/forms/ObjectReference.h"
#include "ui/utils/typical_tableview_config.h"
#include "../ObjectReferenceReflectedObjectsModel.h"

namespace ui::reference::fragments {
   void reflected_refs::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      this->model    = new model_type(controls.view);
      
      controls.view->setModel(this->model);
      ui::typical_tableview_config(controls.view);
   }
   void reflected_refs::load(loaded_form_type& form) {
      this->stub = &form.stub;

      this->model->setSubject(&form.stub);
      if (!is_placed_water(form))
         this->controls.groupbox->setEnabled(false);
   }
   void reflected_refs::save(loaded_form_type& form) {
   }
   bool reflected_refs::is_placed_water(loaded_form_type& form) {
      dovah::form_stub* base_form = form.base_form.get_form_stub();
      if (!base_form)
         return false;
      return dovah::form_stub_helpers::get_activator_water_type(*base_form) != nullptr;
   }
}