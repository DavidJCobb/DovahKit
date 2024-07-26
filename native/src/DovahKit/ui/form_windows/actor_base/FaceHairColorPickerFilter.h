#pragma once
#include <optional>
#include "dovah/data/sex.h"
#include "dovah/forms/Race.h"
#include "widgets/widget-data/DKFormPickerCustomFilter.h"

namespace impl {
   class FaceHairColorPickerFilter final : public DKFormPickerCustomFilter {
      public:
         using DKFormPickerCustomFilter::DKFormPickerCustomFilter;

         virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

      public:
         void setRequiredRace(dovah::form_stub*);
         void setRequiredSex(dovah::sex);

      protected:
         dovah::loaded_form_ptr<dovah::loaded_forms::Race> _race;
         dovah::sex _sex = dovah::sex::male;
   };
}