#pragma once
#include <stdexcept>

namespace cobb::bitstreams::exceptions {
   // Thrown when trying to stream a container that contains too many elements. 
   // Containers are serialized with a size prefix, and the prefix must have a 
   // consistent bitcount, which thereby limits the max element count.
   class container_length_too_large : public std::exception {
   };
}
