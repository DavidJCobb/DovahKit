#pragma once
#include "./impl/modifier_wrapper.h"

namespace lu::bytestreams {
   template<typename... Modifiers>
   class apply_modifiers {
      private:
         apply_modifiers() = delete;
         ~apply_modifiers() = delete;

      public:
         template<typename T>
            requires (!impl::is_modifier_wrapper<T>)
         static constexpr impl::modifier_wrapper<T, Modifiers...>&& to(T& dst) {
            return impl::modifier_wrapper<T, Modifiers...>(dst);
         }
   };

   #pragma region Modifier types
      class opposite_endianness {};
   #pragma endregion
}