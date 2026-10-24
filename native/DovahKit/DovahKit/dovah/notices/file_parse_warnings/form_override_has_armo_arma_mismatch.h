#pragma once
#include <cstdint>
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
   enum class form_type : uint8_t;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class form_override_has_armo_arma_mismatch final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr form_override_has_armo_arma_mismatch(
            const std::string& overriding_file,
            bare_form_id       overriding_record_id,
            bare_form_id       overriding_form_id,
            const std::string  overridden_file,
            form_stub&         overridden_form
         ) :
            base_file_parse_warning(overriding_file),
            overriding_form({
               overriding_file,
               overriding_record_id,
               overriding_form_id,
            }),
            overridden_form({
               overridden_file,
               overridden_form,
            })
         {}

      public:
         struct {
            std::string source_file;
            form_stub&  stub;
         } overridden_form;
         struct {
            std::string  source_file;
            bare_form_id record_id = 0;
            bare_form_id form_id   = 0;
         } overriding_form;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"