#include "./TopicInfoSharedInfoFormFilter.h"
#include "helpers/string/stricontains_ascii.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/Topic.h"

/*virtual*/ bool TopicInfoSharedInfoFormFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (stub.form_type != dovah::form_type::topic_info)
      return false;

   if (!this->_filter.empty()) {
      if (!cobb::stricontains_ascii(stub.editorID, this->_filter))
         return false;
   }

   auto* topic = stub.get_parent_form();
   if (!topic || topic->form_type != dovah::form_type::topic)
      return false;
   if (dovah::form_stub_helpers::get_dialogue_topic_quest(topic) != this->_quest)
      return false;
   if (dovah::form_stub_helpers::get_dialogue_topic_branch(topic))
      return false;

   {
      auto loaded = topic->load().ptr_cast<dovah::loaded_forms::Topic>();
      if (!loaded)
         return false;
      if (loaded->subtype != 'IDAT')
         return false;
   }
   return true;
}

void TopicInfoSharedInfoFormFilter::setEditorIDFilter(const std::string_view v) {
   if (v == this->_filter)
      return;
   this->_filter = v;
   this->_refilter_all_forms();
}
void TopicInfoSharedInfoFormFilter::setOwningQuest(dovah::form_stub* stub) {
   if (stub == this->_quest)
      return;
   if (stub && stub->form_type != dovah::form_type::quest)
      return;
   this->_quest = stub;
   this->_refilter_all_forms();
}