#pragma once
#include <cstdint>
#include <vector>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::topic_info {
   //
   // Every response in a TopicInfo needs to have a unique ID, which will be written 
   // into the filename for any associated voicelines. ID zero, however, causes a 
   // response to use the filename "New Response" in place of a substring that would 
   // uniquely identify the quest, topic, info, and response number.
   //
   class response_has_id_zero : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr response_has_id_zero(
            form_stub& subject,
            size_t num_responses_using
         )
         :
            base_form_load_warning(subject),
            num_responses_using(num_responses_using)
         {}

         size_t num_responses_using = 0;
   };
}
#include "../../../_util.undef.h"