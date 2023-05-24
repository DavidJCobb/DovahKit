#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>
#include "../streams/bitstream_position.h"
#include "./exceptions/read_exception.h"
#include "./_base.h"
#include "./bitstreamable_struct.h"
#include "./bitstreamable_container.h"
#include "./bitstreamable_primitive.h"
#include "./data_header.h"
#include "./override_bitcount.h"

// streamable types:
#include "../eight_cc.h"

class QString;

namespace cobb::bitstreams {
   class reader : public _bitstream_base {
      public:
         using header_type   = data_header;
         using position_type = cobb::streams::bitstream_position;

         using buffer_type = const std::uint8_t*;
         using size_type   = size_t;
         
      protected:
         buffer_type   _buffer   = nullptr; // unowned
         size_type     _size     = 0; // in bytes
         position_type _position = {};
         header_type   _header   = {}; // automatically read when the buffer is set

      protected:
         constexpr void _consume_byte(uint8_t& out, size_t bitcount, int& consumed); // reads {std::min(std::min(8, bitcount), (8 - this->_position.bits))} bits from the buffer
         constexpr void _read_header();

      public:
         constexpr reader() {}
         constexpr reader(buffer_type b, size_type size_in_bytes);
         constexpr ~reader() {}

         constexpr buffer_type data() const noexcept { return this->_buffer; }
         constexpr size_type size() const noexcept { return this->_size; }
         constexpr bool empty() const noexcept { return this->size() == 0; }

         constexpr position_type get_position() const noexcept { return this->_position; }
         constexpr size_t get_bitpos()   const noexcept { return this->_position.in_bits(); }
         constexpr size_t get_bitshift() const noexcept { return this->_position.bits; }
         constexpr size_t get_bytepos()  const noexcept { return this->_position.bytes; }
         constexpr size_t get_bytespan() const noexcept { return this->_position.bytespan(); }

         constexpr const header_type& header() const noexcept { return this->_header; }

         constexpr size_t bits_remaining() const noexcept;
         constexpr size_t bytes_remaining() const noexcept { return this->size() - this->get_bytespan(); }

         constexpr bool is_at_end() const noexcept { return this->get_bytepos() >= this->size(); }
         constexpr bool is_in_bounds(size_t bytes = 0) const noexcept;
         constexpr bool is_byte_aligned() const noexcept { return this->get_bitshift() == 0; }

         constexpr void require_remaining_bits(size_t) const;  // Throws if not enough bits remain in the stream.
         constexpr void require_remaining_bytes(size_t) const; // Throws if not enough bytes remain in the stream.

         constexpr void set_buffer(buffer_type, size_t size_in_bytes) noexcept;
         constexpr void set_bitpos(size_t b);
         constexpr void set_bytepos(size_t b);

         constexpr void skip_bits(size_t b) { this->set_bitpos(this->get_bitpos() + b); }
         constexpr void skip_bytes(size_t b) { this->skip_bits(b * 8); }

      protected:
         template<typename Exception, typename... Types> requires std::is_base_of_v<exceptions::read_exception, Exception>
         [[noreturn]] constexpr void _throw_exception(Types&&... args) const {
            auto except = Exception(std::forward<Types>(args)...);
            except.where_were_we = this->_where_are_we;
            throw except;
         }

         template<typename T>
         struct _multi_stream_type_adjustor;
         
         template<typename T> requires (!impl::_override_bitcount::readable_specialization<T>)
         struct _multi_stream_type_adjustor<T> {
            using type = std::decay_t<T>&;
         };
         
         template<impl::_override_bitcount::readable_specialization T>
         struct _multi_stream_type_adjustor<T> {
            using type = const std::decay_t<T>&;
         };

      public:
         #pragma region Stream overloads

         // Multi-stream call. You can pass references to multiple fields, and stream all of them 
         // using a single call. Additionally, you can use override_bitcount to wrap any field and 
         // override its bitcount.
         template<typename... Types> requires (sizeof...(Types) > 1)
         constexpr void stream(Types&&... args) {
            (this->stream(std::forward<typename _multi_stream_type_adjustor<Types>::type>(args)), ...);
         }
         //
         template<bitstreamable_primitive... Types> requires (sizeof...(Types) > 1)
         constexpr void unchecked_stream(_multi_stream_type_adjustor<Types>::type... args) {
            (this->unchecked_stream(std::forward<typename _multi_stream_type_adjustor<Types>::type>(args)), ...);
         }
         
         template<bitstreamable_struct T> requires (!std::is_const_v<T>)
         constexpr void stream(T& v) {
            const auto step = add_parse_step_for_typename<T>();
            v.stream(*this);
         }

         template<bitstreamable_primitive T> requires (!std::is_const_v<T>)
         constexpr void stream(T& v);
         //
         template<bitstreamable_primitive T> requires (!std::is_const_v<T>)
         constexpr void unchecked_stream(T& v);

         template<impl::_override_bitcount::readable_specialization Wrapper>
         constexpr void stream(const Wrapper& v) {
            this->require_remaining_bits(Wrapper::bitcount);
            this->unchecked_stream(v);
         }
         //
         template<impl::_override_bitcount::readable_specialization Wrapper>
         constexpr void unchecked_stream(const Wrapper& v);

         template<size_t length_bitcount, bitstreamable_container T> requires (!std::is_same_v<T, QString>)
         constexpr void stream(T& v);

         constexpr void stream(cobb::eight_cc& v);
         constexpr void unchecked_stream(cobb::eight_cc& v);
         #pragma endregion

         [[nodiscard]] constexpr uintmax_t stream_bits(size_t bitcount);
         [[nodiscard]] constexpr uintmax_t unchecked_stream_bits(size_t bitcount);

         template<bitstreamable_primitive T>
         constexpr void stream_bits(size_t bitcount, T&);
         //
         template<bitstreamable_primitive T>
         constexpr void unchecked_stream_bits(size_t bitcount, T&);
   };
}

#include "./reader.inl"