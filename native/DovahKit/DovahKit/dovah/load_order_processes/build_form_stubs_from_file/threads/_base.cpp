#include "./_base.h"
#include "../process.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   namespace impl {
      void base_implementation::_abort() {
         this->_config.process.abort();
      }
      void base_implementation::_finish_stub(form_stub*&& stub, tes::record_view& record) {
         this->_config.process._done_parsing_stub({}, stub, record);
      }
   }
}