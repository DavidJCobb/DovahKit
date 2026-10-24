#pragma once
#include "dovah/tes/record_header.h"
#include "../base_file_parse_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class the_game_doesnt_load_new_actor_value_infos final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr the_game_doesnt_load_new_actor_value_infos(
            const std::string&        source_filename,
            const tes::record_header& record_header,
            size_t                    record_offset
         ) :
            base_file_parse_warning(source_filename),
            record_header(record_header),
            record_offset(record_offset)
         {}

      public:
         tes::record_header record_header;
         size_t             record_offset;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"