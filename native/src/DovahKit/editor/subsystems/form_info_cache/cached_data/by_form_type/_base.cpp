#include "./_base.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/form_stub.h"

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void _base::_read_form_stub_from_subrecord(dovah::form_stub*& dst, dovah::tes_file_reading::subrecord& subrecord, dovah::form_type allowed_type) {
      dovah::form_reference_t ref;
      if (subrecord.read(ref)) {
         auto* stub = dst = ref.get_form_stub();
         if (stub && stub->form_type != allowed_type)
            dst = nullptr;
      }
   }
   void _base::_update_form_stub_from_loaded(dovah::form_stub*& dst, const dovah::form_reference_t& src, dovah::form_type allowed_type) {
      if (auto* stub = src.get_form_stub(); stub && stub->form_type == allowed_type) {
         dst = stub;
      } else {
         dst = nullptr;
      }
   }
}