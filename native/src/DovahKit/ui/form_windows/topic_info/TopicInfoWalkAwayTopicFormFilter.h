#pragma once
#include <QPointer>
#include "widgets/widget-data/DKFormPickerCustomFilter.h"
#include "./TopicInfoLinkedTopicsModel.h"

class TopicInfoLinkedTopicsModel;

class TopicInfoWalkAwayTopicFormFilter final : public DKFormPickerCustomFilter {
   public:
      using DKFormPickerCustomFilter::DKFormPickerCustomFilter;

      virtual bool form_matches(const dovah::form_stub& stub) const noexcept override;

   public:
      void setSourceModel(TopicInfoLinkedTopicsModel*);

   protected:
      QPointer<TopicInfoLinkedTopicsModel> _model = nullptr;
};