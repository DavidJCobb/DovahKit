#pragma once
#include <cstdint>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class partial_info_override_has_different_parent final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         struct {
            bare_form_id_t local_id  = 0;
            bare_form_id_t global_id = 0;
            const enum form_type form_type = form_type::topic_info;
         } record;
         form_stub*  parent_of_overriding = nullptr;
         form_stub*  parent_of_overridden = nullptr;
         std::string source_file_for_overridden;
   };
}
#include "../_util.undef.h"