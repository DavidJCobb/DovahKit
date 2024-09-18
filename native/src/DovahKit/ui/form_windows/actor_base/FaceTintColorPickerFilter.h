#pragma once
#include <cstdint>
#include <optional>
#include "dovah/data/sex.h"
#include "dovah/forms/Race.h"
#include "widgets/widget-data/DKCustomFormFilter.h"

namespace impl {
   class FaceTintColorPickerFilter final : public DKCustomFormFilter {
      public:
         using DKCustomFormFilter::DKCustomFormFilter;

         virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

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