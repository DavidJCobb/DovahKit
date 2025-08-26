#pragma once
#include "widgets/widget-data/DKCustomFormFilter.h"

class PackageTemplatePickerFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      // Ensure that we cannot, through shenanigans or out-of-date form-info-cache 
      // stuff, set a package as its own template.
      void set_exclusion(dovah::form_stub* exclude);

   protected:
      dovah::form_stub* _exclude = nullptr;
};