#pragma once
#include <vector>
#include "widgets/widget-data/DKCustomFormFilter.h"

class IdleNewActionRootPickerFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      void setActions(std::vector<dovah::form_stub*>&&);
      void setActions(const std::vector<dovah::form_stub*>&);

   protected:
      std::vector<dovah::form_stub*> actions_to_exclude;
};