#include "./HeadPartPickerFilter.h"
#include "editor/subsystems/form_info_cache/core.h"

/*virtual*/ bool HeadPartPickerFilter::form_matches(const dovah::form_stub& stub) const noexcept /*override*/ {
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto* info = fic.get_head_part_info(stub);
   if (!info)
      return true;
   if (this->_params.type.has_value()) {
      if (info->type != this->_params.type.value())
         return false;
   }
   if (info->sex.has_value()) {
      if (this->_params.sex != info->sex.value())
         return false;
   }
   if (auto* race_list = info->race_list) {
      //
      // HACK: If the race FLST refers to our required race, then assume that 
      //       our race is in the FLST. This isn't necessarily true -- AFAIK 
      //       it's possible to attach Papyrus data to FLST forms, albeit not 
      //       via the Creation Kit -- but it'll do for now.
      //
      bool found = false;
      for (auto& pair : race_list->outbound) {
         if (pair.second.other == this->_params.race) {
            found = true;
            break;
         }
      }
      if (!found)
         return false;
   }
   return true;
}

void HeadPartPickerFilter::setRequiredRace(dovah::form_stub* v) {
   auto& dst = this->_params.race;
   if (dst == v)
      return;
   dst = v;
   this->_refilter_all_forms();
}
void HeadPartPickerFilter::setRequiredSex(dovah::sex v) {
   auto& dst = this->_params.sex;
   if (dst == v)
      return;
   dst = v;
   this->_refilter_all_forms();
}
void HeadPartPickerFilter::setRequiredType(std::optional<head_part_type> v) {
   auto& dst = this->_params.type;
   if (dst == v)
      return;
   dst = v;
   this->_refilter_all_forms();
}