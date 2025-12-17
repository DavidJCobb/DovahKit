#pragma once
#include <vector>
#include "dovah/data/story_manager.h"
#include "widgets/widget-data/DKCustomFormFilter.h"

class StoryManagerQuestNodeAddQuestsFilter final : public DKCustomFormFilter {
   public:
      using DKCustomFormFilter::DKCustomFormFilter;
         
      virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

      void setEvent(dovah::story_event_code_t);

      void setQuestsToExclude(std::vector<dovah::form_stub*>&&);
      void setQuestsToExclude(const std::vector<dovah::form_stub*>&);

   protected:
      dovah::story_event_code_t event = 0;
      std::vector<dovah::form_stub*> quests_to_exclude;
};