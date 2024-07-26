#pragma once
#include <cstdint>
#include <optional>
#include "dovah/data/sex.h"
#include "dovah/forms/Race.h"
#include "widgets/widget-data/DKFormPickerCustomFilter.h"

namespace impl {
   class FaceTintColorPickerFilter final : public DKFormPickerCustomFilter {
      public:
         using DKFormPickerCustomFilter::DKFormPickerCustomFilter;

         virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

      public:
         void setRequiredRace(dovah::form_stub*);
         void setRequiredSex(dovah::sex);
         void setFaceTintIndex(uint16_t);

      protected:
         dovah::loaded_form_ptr<dovah::loaded_forms::Race> _race;
         dovah::sex _sex = dovah::sex::male;
         uint16_t _tint_index = 0;
   };
}