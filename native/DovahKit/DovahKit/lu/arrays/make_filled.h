#pragma once
#include <array>

namespace lu::arrays {
   namespace impl {
      template<typename ValueType, typename IS, typename... Args>
      struct _make_filled;

      template<typename ValueType, typename... Args, size_t... I>
      struct _make_filled<ValueType, std::index_sequence<I...>, Args...> {
         using result_type = std::array<ValueType, sizeof...(I)>;
         static constexpr result_type make(Args&&... args) {
            return result_type{
               ((void)I, (ValueType{ std::forward<Args>(args)... }))...
            };
         }
      };
   }

   template<typename ValueType, size_t Size, typename... Args>
   constexpr std::array<ValueType, Size> make_filled(Args&&... args) {
      return impl::_make_filled<ValueType, std::make_index_sequence<Size>, Args...>::make(std::forward<Args>(args)...);
   }
}
