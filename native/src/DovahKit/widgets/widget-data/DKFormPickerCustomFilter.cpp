#include "./DKFormPickerCustomFilter.h"
#include <cassert>
#include "../widget-models/DKFormPicker/DKFormPickerModel.h"

void DKFormPickerCustomFilter::_refilter_form(const dovah::form_stub& stub) {
   for (auto& model : this->_models)
      if (model)
         model->forceRecheckFilterOn(stub);
}
void DKFormPickerCustomFilter::_refilter_all_forms() {
   for (auto& model : this->_models)
      if (model)
         model->forceRefill();
}

void DKFormPickerCustomFilter::_hookToModel(ui::impl::DKFormPicker::Model* model) {
   auto it = std::find(this->_models.begin(), this->_models.end(), model);
   if (it != this->_models.end())
      return;
   this->_models.push_back(model);
}
void DKFormPickerCustomFilter::_unhookFromModel(ui::impl::DKFormPicker::Model* model) {
   auto it = std::find(this->_models.begin(), this->_models.end(), model);
   if (it != this->_models.end())
      this->_models.erase(it);
}