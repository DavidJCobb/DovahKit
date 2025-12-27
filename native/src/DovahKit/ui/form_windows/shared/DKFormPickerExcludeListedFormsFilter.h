#pragma once
#include <vector>
#include "widgets/widget-data/DKCustomFormFilter.h"
namespace dovah {
   class form_stub;
}

//
// Exclude all forms in a given list of forms.
//
class DKFormPickerExcludeListedFormsFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      void add_exclusion(dovah::form_stub&);
      void set_exclusion(std::vector<dovah::form_stub*>&&);
      void set_exclusion(const std::vector<dovah::form_stub*>&);

   protected:
      std::vector<dovah::form_stub*> _exclude;
};