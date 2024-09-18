#pragma once
#include "widgets/widget-data/DKCustomFormFilter.h"

class TopicInfoSharedInfoFormFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;

      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      void setEditorIDFilter(const std::string_view);
      void setOwningQuest(dovah::form_stub*);

   protected:
      std::string       _filter;
      dovah::form_stub* _quest = nullptr;
};