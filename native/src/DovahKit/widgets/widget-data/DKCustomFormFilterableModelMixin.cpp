#include "./DKCustomFormFilterableModelMixin.h"

DKCustomFormFilter* DKCustomFormFilterableModelMixin::get_custom_filter() const {
   return this->_custom_filter.data();
}
void DKCustomFormFilterableModelMixin::set_custom_filter(DKCustomFormFilter* filter) {
   if (filter == this->_custom_filter)
      return;
   if (auto* prior = (DKCustomFormFilter*)this->_custom_filter) {
      prior->_unhook_from_model(this);
   }
   this->_custom_filter = filter;
   if (filter)
      filter->_hook_to_model(this);
   this->recheck_custom_filter_for_all_forms();
}