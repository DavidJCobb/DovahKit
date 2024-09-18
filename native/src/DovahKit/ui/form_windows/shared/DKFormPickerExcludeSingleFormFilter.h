#pragma once
#include "widgets/widget-data/DKCustomFormFilter.h"

namespace dovah {
   class form_stub;
}

//
// Filter a DKFormPicker to exclude a single, specific form. Suitable for cases 
// where a form can refer to one other form of the same type ("parent," "inherit 
// from," etc.), but cannot be allowed to refer to itself.
//
class DKFormPickerExcludeSingleFormFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      void set_exclusion(dovah::form_stub* exclude);

   protected:
      dovah::form_stub* _exclude = nullptr;
};