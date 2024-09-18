#pragma once
#include <optional>
#include "dovah/data/sex.h"
#include "dovah/forms/Race.h"
#include "widgets/widget-data/DKCustomFormFilter.h"

namespace impl {
   class FaceHairColorPickerFilter final : public DKCustomFormFilter {
      public:
         using DKCustomFormFilter::DKCustomFormFilter;

         virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      public:
         void setRequiredRace(dovah::form_stub*);
         void setRequiredSex(dovah::sex);

      protected:
         dovah::loaded_form_ptr<dovah::loaded_forms::Race> _race;
         dovah::sex _sex = dovah::sex::male;
   };
}