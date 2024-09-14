#pragma once
#include "widgets/widget-data/DKFormPickerCustomFilter.h"

class DialogueBranchStartingTopicFormFilter final : public DKFormPickerCustomFilter {
   public:
      using DKFormPickerCustomFilter::DKFormPickerCustomFilter;

      virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

   public:
      void setDialogueBranch(const dovah::form_stub*);

   protected:
      const dovah::form_stub* branch = nullptr;
};