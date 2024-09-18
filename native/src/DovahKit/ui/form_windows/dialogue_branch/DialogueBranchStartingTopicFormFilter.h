#pragma once
#include "widgets/widget-data/DKCustomFormFilter.h"

class DialogueBranchStartingTopicFormFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;

      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      void setDialogueBranch(const dovah::form_stub*);

   protected:
      const dovah::form_stub* branch = nullptr;
};