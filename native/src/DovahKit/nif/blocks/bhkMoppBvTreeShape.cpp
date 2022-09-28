#include "bhkMoppBvTreeShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkMoppBvTreeShape::parse(file_reader& reader) {
      reader.read_ref(this->subject);
      reader.read(this->pad04);
      reader.read(this->scale);
      
      uint32_t mopp_bytecount;
      reader.read(mopp_bytecount);
      this->mopp.data.resize(mopp_bytecount);
      if (reader.version() >= file_version::from_parts<10, 1, 0, 0>) {
         reader.read(this->mopp.origin);
         reader.read(this->mopp.scale);
      }
      if (reader.user_version<2>() >= 34) {
         reader.read(this->mopp.build_type);
      }
      reader.read_vector_contents(this->mopp.data);
   }
}