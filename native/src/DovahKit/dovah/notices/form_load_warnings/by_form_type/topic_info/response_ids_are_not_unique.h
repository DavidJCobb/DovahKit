#pragma once
#include <cstdint>
#include <vector>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::topic_info {
   //
   // Every response in a TopicInfo needs to have a unique ID, which will be written 
   // into the filename for any associated voicelines. There are a limited pool of 
   // IDs available. Renumbering responses is dicey because a mod could theoretically 
   // reorder responses within an info while still expecting to be able to use the 
   // original voice files.
   //
   class response_ids_are_not_unique : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr response_ids_are_not_unique(
            form_stub& subject,
            size_t response_count,
            std::vector<uint8_t> reused_ids
         )
         :
            base_form_load_warning(subject),
            reused_ids(reused_ids),
            response_count(response_count)
         {}

         size_t response_count = 0;
         std::vector<uint8_t> reused_ids;
   };
}
#include "../../../_util.undef.h"