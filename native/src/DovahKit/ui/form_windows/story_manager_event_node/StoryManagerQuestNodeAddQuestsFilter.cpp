#include "./StoryManagerQuestNodeAddQuestsFilter.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/quest.h"
#include "editor/subsystems/form_info_cache/core.h"

/*virtual*/ bool StoryManagerQuestNodeAddQuestsFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto* info = fic.get_quest_info(stub);
   if (!info)
      return false;
   if (info->event_signature != this->event)
      return false;

   auto it = std::find(this->quests_to_exclude.begin(), this->quests_to_exclude.end(), &stub);
   return (it == this->quests_to_exclude.end());
}

void StoryManagerQuestNodeAddQuestsFilter::setEvent(dovah::story_event_code_t event) {
   if (event == this->event)
      return;
   this->event = event;
   this->_refilter_all_forms();
}

void StoryManagerQuestNodeAddQuestsFilter::setQuestsToExclude(std::vector<dovah::form_stub*>&& list) {
   this->quests_to_exclude = std::move(list);
   this->_refilter_all_forms();
}
void StoryManagerQuestNodeAddQuestsFilter::setQuestsToExclude(const std::vector<dovah::form_stub*>& list) {
   this->quests_to_exclude = list;
   this->_refilter_all_forms();
}