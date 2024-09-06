#include "./DKFormListPaneCustomFilter.h"
#include <cassert>
#include "../widget-models/DKFormListPaneModel.h"

void DKFormListPaneCustomFilter::_refilter_form(dovah::form_stub& stub) {
   for (auto& model : this->_models)
      if (model)
         model->forceRecheckFilterOn(stub);
}
void DKFormListPaneCustomFilter::_refilter_all_forms() {
   for (auto& model : this->_models)
      if (model)
         model->forceRecheckFilter();
}

void DKFormListPaneCustomFilter::_hookToModel(DKFormListPaneModel* model) {
   auto it = std::find(this->_models.begin(), this->_models.end(), model);
   if (it != this->_models.end())
      return;
   this->_models.push_back(model);
}
void DKFormListPaneCustomFilter::_unhookFromModel(DKFormListPaneModel* model) {
   auto it = std::find(this->_models.begin(), this->_models.end(), model);
   if (it != this->_models.end())
      this->_models.erase(it);
}