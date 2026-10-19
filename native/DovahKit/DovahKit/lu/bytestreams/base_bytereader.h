#pragma once
#include <bit>
#include "../concepts/instantiation_of/std_array.h"
#include "../memory/buffer_view.h"
#include "./concepts/can_read_struct.h"
#include "./concepts/serializable_scalar.h"
#include "./concepts/unchecked_read_is_noexcept.h"
#include "./apply_modifiers.h"
#include "../macros/FORCE_INLINE.define.h"

namespace lu::bytestreams {
   namespace impl {
      template<typename T>
      concept serializable_scalar_or_array_thereof = concepts::serializable_scalar<T> || requires {
         requires lu::concepts::instantiation_of_std_array<T>;
         requires concepts::serializable_scalar<typename T::value_type>;
      };

      template<typename T, typename Stream>
      concept readable_value_sans_modifiers = concepts::can_read_struct<T, Stream> || serializable_scalar_or_array_thereof<T>;

      // Needed for multi-read member functions. Enforces the above two concepts for types `T&` and `U` respectively.
      template<typename T, typename Stream>
      concept readable_value_reference =
         requires {
            requires std::is_lvalue_reference_v<T>;
            requires readable_value_sans_modifiers<std::remove_reference_t<T>, Stream>;
         } ||
         requires {
            requires is_modifier_wrapper<T>;
            requires readable_value_sans_modifiers<typename T::type, Stream>;
         }
      ;
   }

   class base_bytereader {
      protected:
         memory::buffer_view _view;
         size_t _position       = 0;
         bool   _needs_byteswap = false;
         #if _DEBUG
            bool _is_in_struct_read = false;
         #endif

      protected:
         template<typename Self, impl::serializable_scalar_or_array_thereof T>
         LU_FORCE_INLINE void _copy_bytes(this Self& self, T& dst);

         template<impl::serializable_scalar_or_array_thereof T>
         LU_FORCE_INLINE static constexpr void _byteswap(T& dst);

         template<typename Self, impl::serializable_scalar_or_array_thereof T, typename ModifiersFrom>
         constexpr void _unchecked_read_impl(this Self& self, T& dst);

         template<typename Self, typename T>
         LU_FORCE_INLINE constexpr void _invoke_struct_unchecked_read_func(this Self& self, T& dst)
            noexcept(concepts::unchecked_read_is_noexcept<T, Self>);

         template<typename Self, concepts::can_read_struct<Self> T, typename ModifiersFrom>
         constexpr void _unchecked_read_impl(this Self& self, T& dst)
            noexcept(concepts::unchecked_read_is_noexcept<T, Self>);

         template<typename Self, typename T, typename ModifiersFrom>
         [[nodiscard]] constexpr bool _read_impl(this Self& self, T& dst);

         #pragma region Template machinery for multi-reads
            #pragma region Unchecked multi-reads, by runs
               //
               // Read one or more values in a row, where all of the values to be read are either scalars 
               // or structs, but not a mix of the two.
               //
               template<bool... HandleThisArg, typename Self, impl::readable_value_reference<Self>... Ts>
               LU_FORCE_INLINE constexpr void _unchecked_multi_read_of_run(this Self& self, Ts&&... dst);

               //
               // Given a list of arguments to read, divide them into runs of the same "kind" (serializable 
               // scalar or serializable struct), and then read them using one function call per run. This 
               // allows us to batch together certain checks and handling.
               // 
               // `End` parameter needed for nested invocation from the "checked" functions. For unchecked, 
               // set `End` to `sizeof...(Ts)`.
               //
               template<size_t StartFrom, size_t End, typename Self, impl::readable_value_reference<Self>... Ts>
               LU_FORCE_INLINE constexpr void _unchecked_multi_read_in_runs(this Self& self, Ts&&... dst);
            #pragma endregion

            #pragma region Checked multi-reads, by runs
               //
               // Read one or more values in a row, where all of the values to be read either have, or do not 
               // have, statically known serialized sizes.
               //
               template<bool... HandleThisArg, typename Self, impl::readable_value_reference<Self>... Ts>
               [[nodiscard]] LU_FORCE_INLINE constexpr bool _checked_multi_read_of_run(this Self& self, Ts&&... dst);

               //
               // Given a list of arguments to read, divide them into runs based on which ones have a 
               // statically known serialized size, and then read them using one function call per run. This 
               // allows us to batch together certain checks and handling.
               //
               template<size_t StartFrom, typename Self, impl::readable_value_reference<Self>... Ts>
               [[nodiscard]] LU_FORCE_INLINE constexpr bool _checked_multi_read_in_runs(this Self& self, Ts&&... dst);
            #pragma endregion

            #pragma region Convert integer sequences to NTTP packs
               template<typename Sequence>
               struct _multi_read_of_run_invoker;
               //
               template<bool... HandleThisArg>
               struct _multi_read_of_run_invoker<std::integer_sequence<bool, HandleThisArg...>> {
                  template<typename Self, impl::readable_value_reference<Self>... Args>
                  LU_FORCE_INLINE static void invoke_unchecked(Self& self, Args&&... args);

                  template<typename Self, impl::readable_value_reference<Self>... Args>
                  [[nodiscard]] LU_FORCE_INLINE static bool invoke_checked(Self& self, Args&&... args);
               };
            #pragma endregion
         #pragma endregion

      public:
         constexpr void set_buffer(memory::buffer_view v);
         constexpr void set_needs_endianness_swap(bool v); // note: calling this from inside a struct's read function is UB

         [[nodiscard]] constexpr bool is_in_bounds() const noexcept;
         [[nodiscard]] constexpr bool is_in_bounds(size_t bytecount) const noexcept;

         [[nodiscard]] constexpr const void* data_at_position() const noexcept;
         [[nodiscard]] constexpr size_t position() const noexcept;
         constexpr void seek(size_t); // can throw `std::out_of_range`
         constexpr void unchecked_seek(size_t);

         #pragma region read(...)
            template<typename Self, impl::readable_value_sans_modifiers<Self> T>
            constexpr bool read(this Self& self, T& dst);

            template<typename Self, impl::is_modifier_wrapper T>
            constexpr bool read(this Self& self, T dst);

            // multi-read
            template<typename Self, impl::readable_value_reference<Self>... Ts>
               requires (sizeof...(Ts) > 1)
            LU_FORCE_INLINE constexpr bool read(this Self& self, Ts&&... dst);
         #pragma endregion

         bool read_bytes(this auto& self, void* dst, size_t dst_size);

         constexpr void skip_bytes(size_t s);

         #pragma region unchecked_read(...)
            template<typename Self, impl::readable_value_sans_modifiers<Self> T>
            constexpr void unchecked_read(this Self& self, T& dst);

            template<typename Self, impl::is_modifier_wrapper T>
            constexpr void unchecked_read(this Self& self, T dst);

            // multi-read
            template<typename Self, impl::readable_value_reference<Self>... Ts>
               requires (sizeof...(Ts) > 1)
            LU_FORCE_INLINE constexpr void unchecked_read(this Self& self, Ts&&... dst);
         #pragma endregion

         void unchecked_read_bytes(this auto& self, void* dst, size_t dst_size);
   };
}

#include "../macros/FORCE_INLINE.undef.h"
#include "./base_bytereader.inl"