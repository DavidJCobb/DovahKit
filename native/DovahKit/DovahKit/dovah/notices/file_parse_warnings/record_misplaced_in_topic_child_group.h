#pragma once
#include <optional>
#include "dovah/tes/group_header.h"
#include "dovah/tes/record_header.h"
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
   enum class form_type : uint8_t;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   //
   // A non-topic-info form was found inside the group that holds a topic's 
   // child topic-infos.
   //
   class record_misplaced_in_topic_child_group final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr record_misplaced_in_topic_child_group(
            const std::string&          source_filename,
            form_stub&                  topic_stub,
            size_t                      group_offset,
            const tes::group_header&    group_header,
            size_t                      misplaced_record_offset,
            const tes::record_header&   misplaced_record_header,
            form_type                   misplaced_form_type,
            std::optional<bare_form_id> misplaced_form_id = {}
         ) :
            base_file_parse_warning(source_filename),
            topic(topic_stub),
            group({
               group_offset,
               group_header,
            }),
            misplaced_form({
               misplaced_form_id,
               misplaced_form_type,
               misplaced_record_offset,
               misplaced_record_header,
            })
         {}

      public:
         form_stub& topic;
         struct {
            size_t            offset;
            tes::group_header header;
         } group;
         struct {
            std::optional<bare_form_id> form_id;
            enum form_type     form_type;
            size_t             record_offset;
            tes::record_header record_header;
         } misplaced_form;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"