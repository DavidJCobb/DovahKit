#pragma once
#include "widgets/widget-data/DKCustomFormFilter.h"

class InteriorCellPickerFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;
};
