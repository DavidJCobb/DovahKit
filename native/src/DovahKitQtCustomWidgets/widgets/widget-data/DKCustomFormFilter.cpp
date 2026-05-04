#include "./DKCustomFormFilter.h"
#include "./DKCustomFormFilterableModelMixin.h"

void DKCustomFormFilter::_refilter_form(dovah::form_stub& stub) {
   for (auto& model : this->_models)
      if (auto* casted = dynamic_cast<DKCustomFormFilterableModelMixin*>(model.data()))
         casted->recheck_custom_filter_for_form(stub);
}
void DKCustomFormFilter::_refilter_all_forms() {
   for (auto& model : this->_models)
      if (auto* casted = dynamic_cast<DKCustomFormFilterableModelMixin*>(model.data()))
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

   // Real quick, some hygiene, since models we've been hooked to previously may have 
   // been deleted. My main concern is that I don't want the list to grow endlessly in 
   // length with an unbounded number of useless null entries.
   std::erase_if(this->_models, [](auto& ptr) { return ptr == nullptr; });

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

   auto&  list  = this->_models;
   bool   found = false;
   bool   nulls = false;
   for (size_t i = 0; i < list.size(); ++i) {
      if (!found && list[i] == casted) {
         found = true;
         list.erase(list.begin() + i);
         --i;
         continue;
      }
      if (list[i] == nullptr) {
         nulls = true;
      }
   }
   if (nulls) {
      // Some hygiene, since models we've been hooked to previously may have 
      // been deleted.
      std::erase_if(list, [](auto& ptr) { return ptr == nullptr; });
   }
}