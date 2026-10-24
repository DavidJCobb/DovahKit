#pragma once
#include "dovah/tes/group_header.h"
#include "dovah/tes/record_header.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   //
   // Produced for all (recoverable) situations wherein a record is found in the wrong group. 
   // This includes:
   // 
   //  - Non-child records listed as children of some form.
   // 
   //  - Cell child records listed directly in the outer child group rather than one of the 
   //    inner, categorized, child groups.
   // 
   //  - A persistent cell child being in the non-persistent group, or vice versa.
   // 
   //  - A non-persistent ref was placed in any child group of a persistent cell.
   // 
   //  - A non-persistent ref was placed in the child group of a cell that wasn't flagged as 
   //    persistent, but was placed within the file as a persistent cell would be.
   //
   class record_found_in_wrong_group final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr record_found_in_wrong_group(
            const std::string&          source_filename,
            size_t                      group_offset,
            const tes::group_header&    group_header,
            form_stub*                  group_is_for_children_of,
            size_t                      misplaced_record_offset,
            const tes::record_header&   misplaced_record_header,
            form_stub*                  misplaced_form_stub
         ) :
            base_file_parse_warning(source_filename),
            group({
               group_offset,
               group_header,
               group_is_for_children_of,
            }),
            misplaced_form({
               misplaced_record_offset,
               misplaced_record_header,
               misplaced_form_stub,
            })
         {}

      public:
         struct {
            size_t            offset;
            tes::group_header header;
            form_stub*        holds_children_of;
         } group;
         struct {
            size_t             record_offset;
            tes::record_header record_header;
            form_stub*         stub;
         } misplaced_form;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"