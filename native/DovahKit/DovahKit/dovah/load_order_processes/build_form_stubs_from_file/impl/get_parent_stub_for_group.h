#pragma once
#include <cstdint>
#include <optional>
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/tes/file.h"
#include "dovah/tes/group_header.h"
#include "dovah/types/bare_form_id.h"
#include "dovah/utils/form_ids/record_id_to_form_id.h"
#include "../process_params.h"
namespace dovah {
   enum class form_type : uint8_t;
}

#include "dovah/exceptions/file_load_errors/missing_master_detected_at_record_load.h"
#include "dovah/exceptions/file_parse_errors/child_form_group_specifies_bad_parent_id.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::impl {
   template<form_type FormType>
   form_stub& get_parent_stub_for_group(process_params& ip, size_t group_offset, const tes::group_header& group_header) {
      auto& alo         = ip.active_load_order;
      auto& source_file = ip.source_file;

      form_stub* parent_stub = nullptr;
      {
         using exception_type = exceptions::file_parse_errors::child_form_group_specifies_bad_parent_id;
         using exception_code = exception_type::problem_type;

         bare_form_id parent_record_id = group_header.label;
         std::optional<exception_code> code;
         {
            auto result = utils::form_ids::record_id_to_form_id(source_file, parent_record_id);
            if (result.has_value()) {
               bare_form_id form_id = result.value();
               if (form_id == 0) {
                  code = exception_code::form_id_is_zero;
               } else {
                  parent_stub = alo.get_form_stub_collection().get_form_by_id(result.value());
                  if (!parent_stub) {
                     code = exception_code::form_does_not_exist;
                  } else if (parent_stub->form_type != FormType) {
                     code = exception_code::form_is_the_wrong_type;
                  }
               }
            } else {
               code = exception_code::record_id_invalid;
               switch (result.error()) {
                  using enum utils::form_ids::record_id_to_form_id_error;
                  case out_of_bounds:
                     code = exception_code::record_id_is_out_of_bounds;
                     break;
                  case missing_master:
                     throw exceptions::file_load_errors::missing_master_detected_at_record_load(
                        source_file.path(),
                        source_file.header().masters[parent_record_id >> 0x18].master,
                        parent_record_id
                     );
               }
            }
         }
         if (code.has_value()) {
            throw exception_type(
               source_file,
               group_offset,
               group_header,
               code.value(),
               FormType,
               parent_record_id,
               parent_stub
            );
         }
      }
      return *parent_stub;
   }
}