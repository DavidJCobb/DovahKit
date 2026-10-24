#pragma once
#include <type_traits>
#include "../../utils/integral_wrapper.h"

namespace lu::bytestreams::concepts {
   template<typename T, typename Stream>
   concept serializable_scalar = (
      std::is_integral_v<T>
   || std::is_floating_point_v<T>
   || std::is_enum_v<T>
   || (
         utils::integral_wrapper_type<T>
         //
         // Must not offer `stream` or `unchecked_stream` member functions that take a 
         // reference to the stream.
         //
      && !(requires(T& v, Stream& stream) {
            { v.stream(stream) };
         })
      && !(requires(const T& v, Stream& stream) {
            { v.stream(stream) };
         })
      && !(requires(T& v, Stream& stream) {
            { v.unchecked_stream(stream) };
         })
      && !(requires(const T& v, Stream& stream) {
            { v.unchecked_stream(stream) };
         })
      )
   );
}