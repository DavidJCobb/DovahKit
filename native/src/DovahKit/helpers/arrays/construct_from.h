#pragma once
#include <array>
#include <type_traits>
#include <utility>

namespace cobb::arrays {
   namespace impl {
      template<typename src_array_type, typename dst_array_type, typename IS = std::make_index_sequence<std::tuple_size_v<src_array_type>>>
      struct _construct_from;
      template<typename src_array_type, typename dst_array_type, auto... I>
      struct _construct_from<src_array_type, dst_array_type, std::index_sequence<I...>> {
         template<typename... Args>
         static dst_array_type exec(const src_array_type& src, Args&&... args) {
            return dst_array_type{ typename dst_array_type::value_type(src[I], std::forward<Args>(args)...), ... };
         };
      };
      
   }

   template<typename DstElementType, typename SrcElementType, size_t Size, typename... Args>
      requires requires {
         requires std::is_constructible_v<DstElementType, const SrcElementType&, Args...>;
      }
   constexpr std::array<DstElementType, Size> construct_from(const std::array<SrcElementType, Size>& src, Args&&... args) {
      using src_array_type = std::array<SrcElementType, Size>;
      using dst_array_type = std::array<DstElementType, Size>;
      if constexpr (std::is_default_constructible_v<DstElementType>) {
         dst_array_type dst = {};
         for (size_t i = 0; i < Size; ++i)
            dst[i] = DstElementType(src[i], std::forward<Args>(args)...);
         return dst;
      } else {
         return impl::_construct_from<src_array_type, dst_array_type>::exec(src, std::forward<Args>(args)...);
      }
   }
}