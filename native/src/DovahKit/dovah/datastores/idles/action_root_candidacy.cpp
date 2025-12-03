#include "./action_root_candidacy.h"
#include <cassert>
#include "../../files/tes_file_reading/file_loader.h"

namespace dovah::datastores::impl::idles {
   bool action_root_candidacy::operator<(const action_root_candidacy& other) const noexcept {
      if (this->source_file == other.source_file) {
         //
         // Both candidacies originate from the same file. Check which one 
         // comes earlier in that file.
         //
         if (this->offsets.of_record == other.offsets.of_record)
            return this->offsets.of_subrecord < other.offsets.of_subrecord;
         return this->offsets.of_record < other.offsets.of_record;
      }
      //
      // As a cheap hack, treat a nullptr file pointer as being the active file. 
      // This means that the datastore doesn't need to remember the `file_load_order` 
      // and query the active file in order to update a newly-edited idle.
      //
      if (!this->source_file)
         return true;
      if (!other.source_file)
         return false;
      //
      // Check which file comes earlier in the load order.
      //
      const auto& lo = this->source_file->get_load_order();
      return lo.index_of_file(*this->source_file) < lo.index_of_file(*other.source_file);
   }
}