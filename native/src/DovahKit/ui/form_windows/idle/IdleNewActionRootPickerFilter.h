#pragma once
#include <vector>
#include "widgets/widget-data/DKCustomFormFilter.h"

class IdleNewActionRootPickerFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      std::vector<dovah::form_stub*> actions_to_exclude;
};