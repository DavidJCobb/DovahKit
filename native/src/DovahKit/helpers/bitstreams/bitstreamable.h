#pragma once
#include <type_traits>

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace cobb {
   namespace impl::_bitstreamable {
      template<typename T> concept readable = requires(T& x, bitstreams::reader& r) {
         { x.stream(r) };
      };
      template<typename T> concept writable = requires(const T& x, bitstreams::writer& w) {
         { x.stream(w) };
      };
   }

   template<typename T> concept bitstreamable = impl::_bitstreamable::readable<T> && impl::_bitstreamable::writable<T>;
}
