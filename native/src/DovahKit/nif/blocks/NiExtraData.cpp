#include "NiExtraData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiExtraData::parse(file_reader& reader) {
      reader.read_indexed_string(this->name);
      //
      this->next = nullptr;
      if (reader.version() <= max_version_for_linked_list) {
         reader.read_ref(this->next);
      }
   }
}