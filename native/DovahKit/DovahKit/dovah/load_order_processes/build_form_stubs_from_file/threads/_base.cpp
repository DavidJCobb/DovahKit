#include "./_base.h"
#include "../process.h"
#include "dovah/_passkeys/build_use_info_during_load.h"
#include "dovah/form_stub.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   void base_worker_thread::_finish_stub(form_stub*&& stub, tes::record_view& record) {
      this->_config.process._done_parsing_stub({}, stub, record);
   }
   void base_worker_thread::_set_stub_parent(form_stub& stub, form_stub* parent) {
      stub._set_parent_form_one_way({}, parent);
   }
}