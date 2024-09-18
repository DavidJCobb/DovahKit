#pragma once
#include <QPointer>
#include "widgets/widget-data/DKCustomFormFilter.h"
#include "./TopicInfoLinkedTopicsModel.h"

class TopicInfoLinkedTopicsModel;

class TopicInfoLinkedTopicsFormFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;

      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

   public:
      void setModel(TopicInfoLinkedTopicsModel*);
      void setOwningQuest(const dovah::form_stub*);

   protected:
      QPointer<TopicInfoLinkedTopicsModel> _model = nullptr;
      const dovah::form_stub* _quest = nullptr;
};