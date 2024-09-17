#pragma once
#include "widgets/widget-data/DKFormPickerCustomFilter.h"

class TopicInfoSharedInfoFormFilter final : public DKFormPickerCustomFilter {
   public:
      using DKFormPickerCustomFilter::DKFormPickerCustomFilter;

      virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

   public:
      void setEditorIDFilter(const std::string_view);
      void setOwningQuest(dovah::form_stub*);

   protected:
      std::string       _filter;
      dovah::form_stub* _quest = nullptr;
};