#pragma once
#include "../file_parse_error.h"
#include "dovah/form_type.h"
#include "dovah/types/bare_form_id.h"

namespace dovah::exceptions::file_parse_errors {
   class form_override_has_type_mismatch : public file_parse_error {
      public:
         form_override_has_type_mismatch(
            const tes::file& overriding_source_file,
            bare_form_id     form_id,
            size_t           overriding_record_offset,
            bare_form_id     overriding_record_id,
            form_type        overriding_form_type,
            const tes::file& overridden_source_file,
            form_type        overridden_form_type
         ) :
            file_parse_error(overriding_source_file),
            form_id(form_id),
            overriding_form({
               overriding_form_type,
               overriding_record_id,
               overriding_record_offset,
               overriding_source_file
            }),
            overridden_form({
               overridden_form_type,
               overridden_source_file
            })
         {}

      public:
         bare_form_id form_id;
         struct {
            form_type        form_type;
            bare_form_id     record_id;
            size_t           record_offset;
            const tes::file& source_file;
         } overriding_form;
         struct {
            form_type        form_type;
            const tes::file& source_file;
         } overridden_form;
   };
}