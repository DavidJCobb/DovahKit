#include "./winning_record_loads_after.h"
#include <cassert>
#include "../../files/file_load_order.h"
#include "../../form_stub.h"

namespace dovah::form_stub_helpers {
   extern bool winning_record_loads_after(const form_stub& a, const form_stub& b) {
      auto& lo = a.get_owning_load_order();
      if (&lo != &b.get_owning_load_order())
         return false; // not comparable

      auto* a_info = a.get_source_file_info(-1);
      auto* b_info = b.get_source_file_info(-1);
      assert(!!a_info);
      assert(!!b_info);
      assert(!!a_info->pointer);
      assert(!!b_info->pointer);
      if (a_info->pointer == b_info->pointer) {
         //
         // Same file.
         //
         return a_info->offset < b_info->offset;
      }
      return lo.index_of_file(*a_info->pointer) < lo.index_of_file(*b_info->pointer);
   }
}