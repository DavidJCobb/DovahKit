#pragma once
#include "dovah/_passkeys/add_form_stub_source_file.h"
#include "dovah/exceptions/file_load_errors/missing_master_detected_at_record_load.h"
#include "dovah/exceptions/file_parse_errors/record_has_invalid_record_id.h"
#include "dovah/form_stub.h"
#include "dovah/form_type_info.h"
#include "dovah/tes/file.h"
#include "dovah/tes/record_header.h"
#include "dovah/utils/form_ids/record_id_to_form_id.h"
namespace dovah::tes {
   class file;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline form_stub* make_stub_from_record(const tes::file& source_file, size_t record_offset, const tes::record_header& header) {
      using invalid_id_exception = exceptions::file_parse_errors::record_has_invalid_record_id;

      auto expected_form_id = utils::form_ids::record_id_to_form_id(source_file, header.formID);
      if (!expected_form_id.has_value()) {
         invalid_id_exception::problem_type type;

         switch (expected_form_id.error()) {
            case utils::form_ids::record_id_to_form_id_error::out_of_bounds:
               type = invalid_id_exception::problem_type::out_of_bounds;
               break;
            case utils::form_ids::record_id_to_form_id_error::missing_master:
               throw exceptions::file_load_errors::missing_master_detected_at_record_load(
                  source_file.path(),
                  source_file.header().masters[header.formID >> 0x18].master,
                  header.formID
               );
         }
         throw invalid_id_exception(
            source_file,
            record_offset,
            header,
            type
         );
      }
      auto form_id = expected_form_id.value();
      if (form_id == 0) {
         throw invalid_id_exception(
            source_file,
            record_offset,
            header,
            invalid_id_exception::problem_type::zero_is_not_allowed
         );
      }

      auto* stub = new form_stub;
      stub->_add_source_file({}, const_cast<tes::file&>(source_file), (uint32_t)record_offset, (uint32_t)header.flags);
      stub->form_id   = form_id;
      stub->form_type = form_type_info::signature_to_form_type(header.signature);
      return stub;
   }
}