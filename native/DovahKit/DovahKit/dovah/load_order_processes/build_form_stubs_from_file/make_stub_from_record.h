#pragma once
#include "dovah/_passkeys/add_form_stub_source_file.h"
#include "dovah/form_stub.h"
#include "dovah/form_type_info.h"
#include "dovah/tes/record_header.h"
namespace dovah::tes {
   class file;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline form_stub* make_stub_from_record(const tes::file& source_file, size_t record_offset, const tes::record_header& header) {
      auto* stub = new form_stub;
      stub->_add_source_file({}, const_cast<tes::file&>(source_file), record_offset, (uint32_t)header.flags);
      stub->form_id   = header.formID;
      stub->form_type = form_type_info::signature_to_form_type(header.signature);
      return stub;
   }
}