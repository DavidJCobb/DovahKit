#pragma once
#include <stdexcept>
#include <type_traits>
#include "byteswap.h"
#include "endian.h"
#include "type_traits/is_literal.h"
#include "type_traits/is_std_array.h"

namespace cobb {
   namespace impl::generic_reader_ex {
      template<typename T> concept IsLiteralIsh = cobb::is_literal<T> || (std::is_bounded_array_v<T> && cobb::is_literal<std::remove_extent_t<T>>);

      template<typename T> concept OffersReaderHook = requires(T& x, ::cobb::generic_reader_ex& fr) { { x.read(fr) }; };
      template<typename T> concept OffersUncheckedReaderHook = requires(T & x, ::cobb::generic_reader_ex& fr) { { x.unchecked_read(fr) }; };

      template<bool checked, typename T> concept OffersHook = (checked ? OffersReaderHook<T> : OffersUncheckedReaderHook<T>);

      template<bool checked, typename T> concept _AllowSimpleCallList = requires {
         requires cobb::is_std_array<T>;
         typename T::value_type;
         requires IsLiteralIsh<typename T::value_type> || OffersHook<checked, typename T::value_type>;
      };
      template<bool checked, typename T> concept AllowSimpleCall = IsLiteralIsh<T> || _AllowSimpleCallList<checked, T> || OffersHook<checked, T>;
   }

   class generic_reader_ex {
      public:
         class unexpected_end : public std::runtime_error {
            public:
               unexpected_end() : std::runtime_error("Unexpected end-of-stream.") {};
         };

      protected:
         struct {
            const void* data     = nullptr;
            size_t      size     = 0;
            size_t      position = 0;
            std::endian endian   = std::endian::native;
         } state;
         
         inline const void* _at() const noexcept { return (const void*)((std::intptr_t)this->data() + this->position()); }
         
         template<bool checked, typename T> void _fix_endianness(T& v) {
            using namespace impl::generic_reader_ex;
            //
            if constexpr (checked ? OffersReaderHook<T> : OffersUncheckedReaderHook<T>)
               return;
            if constexpr (sizeof(T) == 1)
               return;
            if (this->state.endian == std::endian::native)
               return;
            //
            if constexpr (cobb::is_std_array<T>) {
               for (auto& item : v)
                  _fix_endianness<checked>(item);
               return;
            }
            if constexpr (std::is_bounded_array_v<T> && cobb::is_literal<std::remove_extent_t<T>>) {
               for (size_t i = 0; i < std::extent<T>::value; ++i)
                  v[i] = cobb::byteswap(v[i]);
               return;
            }
            if constexpr (cobb::is_literal<T>) {
               v = cobb::byteswap(v);
               return;
            }
         }
         template<bool checked, typename T> void _read_field(T& v) {
            using namespace impl::generic_reader_ex;
            //
            if constexpr (checked ? OffersReaderHook<T> : OffersUncheckedReaderHook<T>) {
               //
               // Structs can define member functions to handle loading their contents.
               //
               if constexpr (checked) {
                  v.read(*this);
               } else {
                  v.unchecked_read(*this);
               }
               //
               // For this branch, T is a struct that reads its members one by one, and those individual 
               // reads do endianness fixups. As such, we need to return here to avoid doing an (incorrect) 
               // endianness fixup on the struct as a whole.
               //
               return;
            }
            if constexpr (cobb::is_std_array<T>) {
               using V = T::value_type;
               if constexpr (checked) {
                  this->require_size(sizeof(V) * v.size());
               }
               if constexpr (!OffersHook<checked, V>) {
                  this->unchecked_read(v.data(), sizeof(V) * v.size());
               } else {
                  //
                  // The item type relies on a hook (i.e. it supplies its own read/unchecked_read functions), 
                  // so we have to make sure we use the right one by passing the "checked" template parameter.
                  //
                  for (auto& item : v)
                     this->_read_field<checked>(item);
                  return;
               }
               // ...and fall through to endianness check.
            } else if constexpr (std::is_bounded_array_v<T> && cobb::is_literal<std::remove_extent_t<T>>) {
               constexpr auto size = std::extent<T>::value;
               using V = std::remove_extent_t<T>;
               //
               if constexpr (checked) {
                  this->require_size(sizeof(V) * size);
               }
               for (size_t i = 0; i < size; ++i)
                  this->_read_field<false>(v[i]);
               // ...and fall through to endianness check.
            } else if constexpr (cobb::is_literal<T>) {
               if constexpr (checked) {
                  this->require_size(sizeof(T));
               }
               this->unchecked_read(&v, sizeof(T));
               // ...and fall through to endianness check.
            }
            this->_fix_endianness<checked>(v);
         }

      public:
         generic_reader_ex(const void* d, size_t s) : state({ d, s }) {}

         inline const void* data() const noexcept { return this->state.data; }
         inline size_t size() const noexcept { return this->state.size; }
         inline size_t position() const noexcept { return this->state.position; }
         inline bool empty() const noexcept { return this->state.data == nullptr || this->state.size == 0; }
         
         inline void set_endianness(std::endian e) {
            this->state.endian = e;
         }

         inline const void* data_at(size_t p) const noexcept {
            if (p > this->state.size)
               return nullptr;
            if (this->state.data == nullptr)
               return nullptr;
            return (const void*)((std::intptr_t)this->data() + p);
         }

         inline bool at_end() const noexcept { return this->position() == this->size(); }
         inline bool is_in_bounds(size_t s) const noexcept { return this->position() + s <= this->size(); }

         inline void skip(size_t s) {
            auto& p = this->state.position;
            p += s;
            if (p >= this->size())
               p = this->size();
         }
         void require_size(size_t s) {
            if (!this->is_in_bounds(s))
               throw unexpected_end();
         }

         void seek_to(size_t s) {
            if (s >= this->size())
               throw unexpected_end();
            this->state.position = s;
         }

         //
         // Read a value, or throw an exception on an early end-of-stream.
         //
         #pragma region read
         inline void read(void* buffer, size_t size) {
            if (!this->is_in_bounds(size))
               throw unexpected_end();
            this->unchecked_read(buffer, size);
         }
         template<typename T> requires impl::generic_reader_ex::AllowSimpleCall<true, T> inline void read(T& field) {
            this->_read_field<true>(field);
         }

         // overload to require a specific endianness, overriding the one specified in the file header
         template<std::endian E, typename T> requires impl::generic_reader_ex::IsLiteralIsh<T> inline void read(T& field) {
            this->require_size(sizeof(T));
            this->unchecked_read<E>(field);
         }

         template<typename S> requires std::convertible_to<S, size_t>
         void read_prefixed_string(std::string& field) {
            S size;
            this->read(size);
            field.resize(size);
            if (size) {
               this->read(field.data(), size);
               if (field.back() == '\00')
                  field.resize(size - 1);
            }
         }
         #pragma endregion

         //
         // Read a value without bothering to check for end-of-stream (i.e. if you've already checked).
         //
         #pragma region unchecked_read
         inline void unchecked_read(void* buffer, size_t size) {
            memcpy(buffer, _at(), size);
            this->state.position += size;
         }
         template<typename T> requires impl::generic_reader_ex::AllowSimpleCall<false, T> inline void unchecked_read(T& field) {
            this->_read_field<false>(field);
         }

         // overload to require a specific endianness, overriding the one specified in the file header
         template<std::endian E, typename T> requires impl::generic_reader_ex::IsLiteralIsh<T> inline void unchecked_read(T& field) {
            if constexpr (sizeof(T) == 1 || E == std::endian::native) {
               return this->unchecked_read(field);
            }
            this->unchecked_read(field);
            field = cobb::endian_cast<E>(field);
         }
         #pragma endregion
   };
}