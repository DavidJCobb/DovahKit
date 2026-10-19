#pragma once
#include "../customization/serialized_bytecount.h"

namespace lu::bytestreams::concepts {
   // If a struct's serialized bytecount is statically known, and if the struct offers 
   // an `unchecked_stream` member function, then we can implement checked reads based 
   // on unchecked reads. Otherwise, the struct must offer a `stream` member function, 
   // which must do checked reads of the struct's contents.
   template<typename T, typename Stream>
   concept checked_reads_can_be_implemented_using_unchecked_reads = requires(Stream& reader, T& dst) {
      { dst.unchecked_stream(reader) };
      requires serialized_bytecount_is_statically_known<Stream, T>;
   };
}
