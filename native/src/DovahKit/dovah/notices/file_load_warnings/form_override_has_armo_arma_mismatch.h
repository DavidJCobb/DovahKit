#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class form_override_has_armo_arma_mismatch final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr form_override_has_armo_arma_mismatch(form_stub& overridden_stub) : overridden_form({ .stub = overridden_stub }) {}

         struct {
            std::string source_file;
            form_stub&  stub;
         } overridden_form;
         struct {
            std::string source_file;
            struct {
               bare_form_id_t local  = 0;
               bare_form_id_t global = 0;
            } form_ids;
         } overriding_form;
   };
}
#include "../_util.undef.h"