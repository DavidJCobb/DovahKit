#include "./FaceHairColorPickerFilter.h"
#include "dovah/form_stub.h"

namespace impl {
   /*virtual*/ bool FaceHairColorPickerFilter::form_matches(const dovah::form_stub& stub) const noexcept /*override*/ {
      if (!this->_race)
         return true;
      auto& head = this->_race->by_sex[this->_sex].head_data;
      if (&stub == head.default_hair_color.get_form_stub())
         return true;
      for (auto& use : head.hair_colors)
         if (&stub == use.get_form_stub())
            return true;
      return false;
   }

   void FaceHairColorPickerFilter::setRequiredRace(dovah::form_stub* v) {
      if (!v) {
         if (!this->_race)
            return;
         this->_race = nullptr;
      } else {
         if (this->_race && &this->_race->stub == v)
            return;
         this->_race = v->load().ptr_cast<dovah::loaded_forms::Race>();
      }
      this->_refilter_all_forms();
   }
   void FaceHairColorPickerFilter::setRequiredSex(dovah::sex v) {
      auto& dst = this->_sex;
      if (dst == v)
         return;
      dst = v;
      this->_refilter_all_forms();
   }
}