#pragma once
#if _DEBUG
   #include <cassert>
#endif
#include <stdexcept>
#include "./base_bytereader.h"
#include "../bit/byteswap_in_place.h"
#include "../concepts/instantiation_of/std_array.h"
#include "../type_aliases/uint_of_size.h"
#include "../utils/bool_sequences/clear_all_from.h"
#include "../utils/bool_sequences/true_run_extents_of.h"
#include "../utils/type_pack_to_bool_run_by_predicate.h"
#include "./customization/serialized_bytecount.h"
#include "./customization/type_has_opposite_endianness.h"
#include "./strip_modifiers.h"
#include "./unwrap_modifiers.h"
#include "../macros/FORCE_INLINE.define.h"

namespace lu::bytestreams {
   namespace impl {
      template<typename Stream, typename T, typename ModifiersFrom = T>
      concept this_read_has_opposite_endianness = (type_has_opposite_endianness<Stream, T>::value != impl::has_modifier<ModifiersFrom, opposite_endianness>);

      #pragma region Divide multi-read arguments into runs of the same kind (scalar/struct)
         template<typename Stream, size_t StartFrom, typename... Ts>
         struct run_of_kind_wrapper { // [10/11/2026] needed to prevent IntelliSense/EDG from choking on the lambda-as-NTTP
            static constexpr auto predicate = []<typename T>() -> bool {
               return serializable_scalar_or_array_thereof<strip_modifiers<std::decay_t<T>>, Stream>;
            };
            using type = utils::type_pack_to_bool_run_by_predicate<predicate, StartFrom, Ts...>;
         };
         //
         template<typename Stream, size_t StartFrom, typename... Ts>
         using run_of_kind = run_of_kind_wrapper<Stream, StartFrom, Ts...>::type;
      #pragma endregion

      #pragma region Divide multi-read arguments into runs of the same static knowledge (size statically known/not)
         template<typename Stream, size_t StartFrom, typename... Ts>
         struct run_of_static_knowledge_wrapper { // [10/11/2026] needed to prevent IntelliSense/EDG from choking on the lambda-as-NTTP
            static constexpr auto predicate = []<typename T>() -> bool {
               return serialized_bytecount_is_statically_known<Stream, strip_modifiers<std::decay_t<T>>>;
            };
            using type = utils::type_pack_to_bool_run_by_predicate<predicate, StartFrom, Ts...>;
         };
         //
         template<typename Stream, size_t StartFrom, typename... Ts>
         using run_of_static_knowledge = run_of_static_knowledge_wrapper<Stream, StartFrom, Ts...>::type;
      #pragma endregion
   }

   template<typename Self, impl::serializable_scalar_or_array_thereof<Self> T>
   LU_FORCE_INLINE void base_bytereader::_copy_bytes(this Self& self, T& dst) {
      constexpr const size_t size = serialized_bytecount<Self, T>::value;

      std::memcpy(&dst, self._view.data_at(self._position), size);
      self._position += size;
   }

   template<typename Self, impl::serializable_scalar_or_array_thereof<Self> T>
   LU_FORCE_INLINE constexpr void base_bytereader::_byteswap(T& dst) {
      if constexpr (std::is_enum_v<T>) {
         dst = (T)std::byteswap((std::underlying_type_t<T>)dst);
      } else if constexpr (lu::utils::integral_wrapper_type<T>) {
         dst.set_from_integral(std::byteswap(dst.as_integral()));
      } else if constexpr (std::is_floating_point_v<T>) {
         using integral_type = lu::uint_of_size<sizeof(T)>;
         integral_type v = std::bit_cast<integral_type>(dst);
         v = std::byteswap(v);
         dst = std::bit_cast<T>(v);
      } else {
         bits::byteswap_in_place(dst);
      }
   }

   // overload: scalars or arrays thereof
   template<typename Self, impl::serializable_scalar_or_array_thereof<Self> T, typename ModifiersFrom>
   constexpr void base_bytereader::_unchecked_read_impl(this Self& self, T& dst) {
      self._copy_bytes(dst);

      constexpr const bool opposite_endianness = impl::this_read_has_opposite_endianness<Self, T, ModifiersFrom>;
      if (self._needs_byteswap != opposite_endianness) {
         _byteswap<Self>(dst);
      }
   }

   template<typename Self, typename T>
   LU_FORCE_INLINE constexpr void base_bytereader::_invoke_struct_unchecked_read_func(this Self& self, T& dst)
      noexcept(impl::unchecked_read_is_noexcept<T, Self>)
   {
      if constexpr (lu::concepts::instantiation_of_std_array<T> && !concepts::serializable_struct<T, Self>) {
         // read each struct in array (recurse to account for nested arrays)
         for (auto& item : dst) {
            self._invoke_struct_unchecked_read_func(item);
         }
      } else {
         // read single struct
         if constexpr (serialized_bytecount_is_statically_known<Self, T>) {
            #if _DEBUG
               const auto pos_prior = self._position;
            #endif
            if constexpr (concepts::checked_reads_can_be_implemented_using_unchecked_reads<T, Self>) {
               dst.unchecked_stream(self);
            } else {
               dst.stream(self);
            }
            #if _DEBUG
               assert((self._position == pos_prior + serialized_bytecount<Self, T>::value) &&
                  "If a struct's serialized size is statically known, the struct's \"stream\" functions must read exactly and always that many bytes." \
               );
            #endif
         } else {
            dst.stream(self);
         }
      }
   }

   // overload: structs or arrays thereof
   template<typename Self, impl::serializable_struct_or_array_thereof<Self> T, typename ModifiersFrom>
   constexpr void base_bytereader::_unchecked_read_impl(this Self& self, T& dst)
      noexcept(impl::unchecked_read_is_noexcept<T, Self>)
   {
      #if _DEBUG
         const bool was_in_struct_read = self._is_in_struct_read;
         self._is_in_struct_read = true;
      #endif
      constexpr const bool opposite_endianness = impl::this_read_has_opposite_endianness<Self, T, ModifiersFrom>;
      if constexpr (opposite_endianness) {
         self._needs_byteswap = !self._needs_byteswap;
      }
      if constexpr (opposite_endianness && !impl::unchecked_read_is_noexcept<T, Self>) {
         try {
            self._invoke_struct_unchecked_read_func(dst);
         } catch (...) {
            self._needs_byteswap = !self._needs_byteswap;
            #if _DEBUG
               self._is_in_struct_read = was_in_struct_read;
            #endif
            throw;
         }
      } else {
         self._invoke_struct_unchecked_read_func(dst);
      }
      if constexpr (opposite_endianness) {
         self._needs_byteswap = !self._needs_byteswap;
      }
      #if _DEBUG
         self._is_in_struct_read = was_in_struct_read;
      #endif
   }

   // `read` deals with modifier-wrappers and then dispatches to `_read_impl`
   template<typename Self, typename T, typename ModifiersFrom>
   LU_FORCE_INLINE constexpr bool base_bytereader::_read_impl(this Self& self, T& dst) {
      if constexpr (serialized_bytecount_is_statically_known<Self, T>) {
         constexpr size_t size = serialized_bytecount<Self, T>::value;
         if (!self.is_in_bounds(size))
            return false;
         self.template _unchecked_read_impl<Self, T, ModifiersFrom>(dst);
         return true;
      } else {
         //
         // All serializable scalars have statically known bytecounts, so this must be a 
         // serializable struct or array thereof.
         //
         if constexpr (lu::concepts::instantiation_of_std_array<T> && !concepts::serializable_struct<T, Self>) {
            using value_type = T::value_type;
            // recurse, to account for nested array types
            for (auto& item : dst)
               if (!self._read_impl<Self, value_type, ModifiersFrom>(item))
                  return false;
            return true;
         } else {
            return dst.stream(self);
         }
      }
   }

   #pragma region Template machinery for multi-reads
      #pragma region Unchecked multi-reads, by runs
         template<bool... HandleThisArg, typename Self, impl::readable_value_reference<Self>... Ts>
         LU_FORCE_INLINE constexpr void base_bytereader::_unchecked_multi_read_of_run(this Self& self, Ts&&... dst) {
            constexpr const bool run_is_of_scalars = (
               (HandleThisArg ? impl::serializable_scalar_or_array_thereof<strip_modifiers<std::decay_t<Ts>>, Self> : true)
               && ...
            );
            constexpr const bool any_endianness_opposite = (
               (HandleThisArg ? impl::this_read_has_opposite_endianness<Self, strip_modifiers<std::decay_t<Ts>>, std::decay_t<Ts>> : false)
               || ...
            );
            constexpr const bool all_endianness_same = (
               (HandleThisArg ? (impl::this_read_has_opposite_endianness<Self, strip_modifiers<std::decay_t<Ts>>, std::decay_t<Ts>> == any_endianness_opposite) : true)
               && ...
            );

            if constexpr (run_is_of_scalars) {
               ((HandleThisArg ? self._copy_bytes(unwrap_modifiers(dst)), 0 : 0), ...);
               if constexpr (all_endianness_same) {
                  if (self._needs_byteswap == !any_endianness_opposite) {
                     ((HandleThisArg ? _byteswap<Self>(unwrap_modifiers(dst)),0 : 0), ...);
                  }
               } else {
                  if (self._needs_byteswap) {
                     ((HandleThisArg && !impl::this_read_has_opposite_endianness<Self, strip_modifiers<std::decay_t<Ts>>, std::decay_t<Ts>> ? _byteswap<Self>(unwrap_modifiers(dst)),0 : 0), ...);
                  } else {
                     ((HandleThisArg && impl::this_read_has_opposite_endianness<Self, strip_modifiers<std::decay_t<Ts>>, std::decay_t<Ts>> ? _byteswap<Self>(unwrap_modifiers(dst)),0 : 0), ...);
                  }
               }
            } else {
               ((HandleThisArg ? self._unchecked_read_impl<Self, strip_modifiers<Ts>, Ts>(unwrap_modifiers(dst)),0 : 0), ...);
            }
         }

         template<size_t StartAt, size_t End, typename Self, impl::readable_value_reference<Self>... Ts>
         LU_FORCE_INLINE constexpr void base_bytereader::_unchecked_multi_read_in_runs(this Self& self, Ts&&... dst) {
            using run         = utils::bool_sequences::clear_all_from<impl::run_of_kind<Self, StartAt, Ts...>, End>;
            using run_extents = utils::bool_sequences::true_run_extents_of<run>;

            // dummy variables for the debugger
            constexpr const auto run_start  = run_extents::start;
            constexpr const auto run_end    = run_extents::end;
            constexpr const auto run_length = run_extents::length;

            if constexpr (run_extents::length > 0) {
               _multi_read_of_run_invoker<run>::invoke_unchecked(self, std::forward<Ts>(dst)...);
               if constexpr (run_extents::end < sizeof...(Ts) && run_extents::end < End) {
                  self._unchecked_multi_read_in_runs<run_extents::end, End>(std::forward<Ts>(dst)...);
               }
            }
         }
      #pragma endregion
         
      #pragma region Checked multi-reads, by runs
         template<bool... HandleThisArg, typename Self, impl::readable_value_reference<Self>... Ts>
         LU_FORCE_INLINE constexpr bool base_bytereader::_checked_multi_read_of_run(this Self& self, Ts&&... dst) {
            using run_traits = utils::bool_sequences::true_run_extents_of<std::integer_sequence<bool, HandleThisArg...>>;

            constexpr const bool all_sizes_statically_known = (
               (!HandleThisArg || serialized_bytecount_is_statically_known<Self, strip_modifiers<std::decay_t<Ts>>>)
               && ...
            );
      
            if constexpr (all_sizes_statically_known) {
               constexpr size_t total_size = (
                  (HandleThisArg ? serialized_bytecount<Self, strip_modifiers<std::decay_t<Ts>>>::value : 0)
                  + ...
               );
               if (!self.is_in_bounds(total_size))
                  return false;
               self._unchecked_multi_read_in_runs<run_traits::start, run_traits::end>(std::forward<Ts>(dst)...);
               return true;
            } else {
               bool valid = true;
               ((HandleThisArg ? (valid = self.read(dst)) : true) && ...);
               return valid;
            }
         }
   
         template<size_t StartFrom, typename Self, impl::readable_value_reference<Self>... Ts>
         LU_FORCE_INLINE constexpr bool base_bytereader::_checked_multi_read_in_runs(this Self& self, Ts&&... dst) {
            using run         = impl::run_of_static_knowledge<Self, StartFrom, Ts...>;
            using run_extents = utils::bool_sequences::true_run_extents_of<run>;

            // dummy variables for the debugger
            constexpr const auto run_start  = run_extents::start;
            constexpr const auto run_end    = run_extents::end;
            constexpr const auto run_length = run_extents::length;

            if constexpr (run_extents::length > 0) {
               bool result = _multi_read_of_run_invoker<run>::invoke_checked(self, std::forward<Ts>(dst)...);
               if (!result)
                  return false;
               if constexpr (run_extents::end < sizeof...(Ts)) {
                  return self._checked_multi_read_in_runs<run_extents::end>(std::forward<Ts>(dst)...);
               }
            }
            return true;
         }
      #pragma endregion

      #pragma region Convert integer sequences to NTTP packs
         template<bool... HandleThisArg>
         template<typename Self, impl::readable_value_reference<Self>... Args>
         LU_FORCE_INLINE static void base_bytereader::_multi_read_of_run_invoker<std::integer_sequence<bool, HandleThisArg...>>::invoke_unchecked(Self& self, Args&&... args) {
            self.template _unchecked_multi_read_of_run<HandleThisArg...>(std::forward<Args>(args)...);
         }

         template<bool... HandleThisArg>
         template<typename Self, impl::readable_value_reference<Self>... Args>
         LU_FORCE_INLINE static bool base_bytereader::_multi_read_of_run_invoker<std::integer_sequence<bool, HandleThisArg...>>::invoke_checked(Self& self, Args&&... args) {
            return self.template _checked_multi_read_of_run<HandleThisArg...>(std::forward<Args>(args)...);
         }
      #pragma endregion
   #pragma endregion

   constexpr void base_bytereader::set_buffer(memory::buffer_view v) {
      this->_view     = v;
      this->_position = 0;
   }
   constexpr void base_bytereader::set_needs_endianness_swap(bool v) {
      #if _DEBUG
         assert(!this->_is_in_struct_read && "Modifying a bytestream's endianness from inside of a struct's `stream`/`unchecked_stream` functions is undefined behavior.");
      #endif
      this->_needs_byteswap = v;
   }

   constexpr bool base_bytereader::is_in_bounds() const noexcept {
      return this->_position < this->_view.size();
   }
   constexpr bool base_bytereader::is_in_bounds(size_t bytecount) const noexcept {
      return this->_position + bytecount <= this->_view.size();
   }

   constexpr const void* base_bytereader::data_at_position() const noexcept {
      return this->_view.data_at(this->_position);
   }
   constexpr size_t base_bytereader::position() const noexcept {
      return this->_position;
   }
   constexpr void base_bytereader::seek(size_t to) {
      if (to > this->_view.size())
         throw std::out_of_range("cannot seek past the end of a bytereader's buffer");
      this->_position = to;
   }
   constexpr void base_bytereader::unchecked_seek(size_t to) {
      this->_position = to;
   }

   #pragma region read(...)
      template<typename Self, impl::readable_value_sans_modifiers<Self> T>
      constexpr bool base_bytereader::read(this Self& self, T& dst) {
         return self.template _read_impl<Self, T, T>(dst);
      }

      template<typename Self, impl::is_modifier_wrapper T>
      constexpr bool base_bytereader::read(this Self& self, T dst) {
         using value_type = strip_modifiers<T>;
         return self.template _read_impl<Self, value_type, T>(dst.get());
      }

      template<typename Self, impl::readable_value_reference<Self>... Ts>
         requires (sizeof...(Ts) > 1)
      LU_FORCE_INLINE constexpr bool base_bytereader::read(this Self& self, Ts&&... dst) {
         return self._checked_multi_read_in_runs<0>(std::forward<Ts>(dst)...);
      }
   #pragma endregion

   bool base_bytereader::read_bytes(this auto& self, void* dst, size_t dst_size) {
      if (!self.is_in_bounds(dst_size))
         return false;
      std::memcpy(dst, self._view.data_at(self._position), dst_size);
      self._position += dst_size;
      return true;
   }

   constexpr void base_bytereader::skip_bytes(size_t s) {
      this->_position += s;
      const auto size = this->_view.size();
      if (this->_position > size)
         this->_position = size;
   }

   #pragma region unchecked_read(...)
      template<typename Self, impl::readable_value_sans_modifiers<Self> T>
      constexpr void base_bytereader::unchecked_read(this Self& self, T& dst) {
         self.template _unchecked_read_impl<Self, T, T>(dst);
      }

      template<typename Self, impl::is_modifier_wrapper T>
      constexpr void base_bytereader::unchecked_read(this Self& self, T dst) {
         self.template _unchecked_read_impl<Self, typename T::type, T>(dst.get());
      }

      template<typename Self, impl::readable_value_reference<Self>... Ts>
         requires (sizeof...(Ts) > 1)
      LU_FORCE_INLINE constexpr void base_bytereader::unchecked_read(this Self& self, Ts&&... dst) {
         self.template _unchecked_multi_read_in_runs<0, sizeof...(Ts)>(std::forward<Ts>(dst)...);
      }
   #pragma endregion

   void base_bytereader::unchecked_read_bytes(this auto& self, void* dst, size_t dst_size) {
      std::memcpy(dst, self._view.data_at(self._position), dst_size);
      self._position += dst_size;
   }
}

#include "../macros/FORCE_INLINE.undef.h"