#pragma once
#include "dovah/tes/record_header.h"
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class form_initial_record_is_partial final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr form_initial_record_is_partial(
            const std::string&          source_filename,
            bare_form_id                form_id,
            const tes::record_header&   record_header,
            size_t                      record_offset,
            bool                        is_injected
         ) :
            base_file_parse_warning(source_filename),
            form_id(form_id),
            record_header(record_header),
            record_offset(record_offset),
            is_injected(is_injected)
         {}

      public:
         bare_form_id       form_id;
         bool               is_injected; // Skyrim will skip loading a record if it's partial, injected, and not an override. So does DovahKit.
         tes::record_header record_header;
         size_t             record_offset;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"