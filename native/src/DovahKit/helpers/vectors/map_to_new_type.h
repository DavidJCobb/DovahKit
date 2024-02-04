#pragma once
#include <vector>

namespace cobb::vectors {
   template<typename DstValueType, typename SrcValueType, typename ConversionFunctor>
   std::vector<DstValueType> map_to_new_type(const std::vector<SrcValueType>& src, ConversionFunctor&& functor) {
      std::vector<DstValueType> dst;
      const size_t size = src.size();
      dst.resize(size);
      for (size_t i = 0; i < size; ++i) {
         functor(src[i], dst[i]);
      }
      return dst;
   }
}