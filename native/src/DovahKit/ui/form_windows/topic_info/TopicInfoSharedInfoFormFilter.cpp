#include "./TopicInfoSharedInfoFormFilter.h"
#include "helpers/string/stricontains_ascii.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/core.h"

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
   if (!dovahkit::subsystems::form_info_cache::core::get().topic_is_sharedinfo_topic(*topic))
      //
      // You can reference a SharedInfo existing in any quest, as long as it belongs to 
      // an IDAT-subtype topic.
      //
      return false;

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