#pragma once
#include "./write_exception.h"

namespace cobb::bitstreams::exceptions {
   // Thrown when trying to stream a container that contains too many elements. 
   // Containers are serialized with a size prefix, and the prefix must have a 
   // consistent bitcount, which thereby limits the max element count.
   class container_length_too_large : public write_exception {
      public:
         container_length_too_large(size_t size, size_t max) : element_count(size), maximum_count(max) {}

         size_t element_count = 0;
         size_t maximum_count = 0;
   };
}
