#include "./DKCustomFormFilter.h"
#include "./DKCustomFormFilterableModelMixin.h"

void DKCustomFormFilter::_refilter_form(dovah::form_stub& stub) {
   for (auto& model : this->_models)
      if (auto* casted = (DKCustomFormFilterableModelMixin*)model.data())
         casted->recheck_custom_filter_for_form(stub);
}
void DKCustomFormFilter::_refilter_all_forms() {
   for (auto& model : this->_models)
      if (auto* casted = (DKCustomFormFilterableModelMixin*)model.data())
         casted->recheck_custom_filter_for_all_forms();
}

void DKCustomFormFilter::_hook_to_model(DKCustomFormFilterableModelMixin* model) {
   auto* casted = dynamic_cast<QObject*>(model);
   if (!casted) {
      #if _DEBUG
         __debugbreak(); // passed-in DKCustomFormFilterableModelMixin instance is not a subclass of QObject; is this intentional?
      #endif
      return;
   }
   auto it = std::find(this->_models.begin(), this->_models.end(), casted);
   if (it != this->_models.end())
      return;
   this->_models.push_back(casted);
}
void DKCustomFormFilter::_unhook_from_model(DKCustomFormFilterableModelMixin* model) {
   auto* casted = dynamic_cast<QObject*>(model);
   if (!casted) {
      #if _DEBUG
         __debugbreak(); // passed-in DKCustomFormFilterableModelMixin instance is not a subclass of QObject; is this intentional?
      #endif
      return;
   }
   auto it = std::find(this->_models.begin(), this->_models.end(), casted);
   if (it != this->_models.end())
      this->_models.erase(it);
}