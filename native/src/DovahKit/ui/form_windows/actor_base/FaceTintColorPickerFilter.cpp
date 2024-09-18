#include "./FaceTintColorPickerFilter.h"
#include "dovah/form_stub.h"

namespace impl {
   /*virtual*/ bool FaceTintColorPickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
      if (!this->_race)
         return true;
      auto& head = this->_race->by_sex[this->_sex].head_data;
      for (auto& tint : head.face_tints) {
         if (tint.index != this->_tint_index)
            continue;

         if (tint.default_color == &stub)
            return true;
         for (auto& preset : tint.presets)
            if (preset.color == &stub)
               return true;
         return false;
      }
      return true;
   }

   void FaceTintColorPickerFilter::setRequiredRace(dovah::form_stub* v) {
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
   void FaceTintColorPickerFilter::setRequiredSex(dovah::sex v) {
      auto& dst = this->_sex;
      if (dst == v)
         return;
      dst = v;
      this->_refilter_all_forms();
   }
   void FaceTintColorPickerFilter::setFaceTintIndex(uint16_t v) {
      auto& dst = this->_tint_index;
      if (dst == v)
         return;
      dst = v;
      this->_refilter_all_forms();
   }
}