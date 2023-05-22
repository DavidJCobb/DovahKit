#pragma once
#include <type_traits>

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace cobb {
   template<typename T> concept bitstream = std::is_same_v<T, bitstreams::reader> || std::is_same_v<T, bitstreams::writer>;
}
