#pragma once
#include <bit>
#include <cstdint>
#include "../streams/bitstream_position.h"
#include "./bitstreamable.h"
#include "./bitstreamable_container.h"
#include "./bitstreamable_primitive.h"
#include "./data_header.h"
#include "./stream_with_bitcount.h"

// streamable types:
#include "../eight_cc.h"

class QString;

namespace cobb::bitstreams {
   class writer {
      public:
         using header_type   = data_header;
         using position_type = cobb::streams::bitstream_position;

         using buffer_type = std::uint8_t*;
         using size_type   = size_t;
         
      protected:
         buffer_type   _buffer   = nullptr; // unowned
         size_type     _size     = 0; // in bytes
         position_type _position = {};
         header_type   _header   = {}; // not automatically written
         
      protected:
         constexpr void _buffer_free();

         constexpr uint8_t& _access_byte(size_t bytepos) const noexcept;
         constexpr void _ensure_room_for(unsigned int bitcount);

      public:
         constexpr writer() {}
         constexpr ~writer();

         constexpr buffer_type data() const noexcept { return this->_buffer; }
         constexpr size_type size() const noexcept { return this->_size; }
         constexpr bool empty() const noexcept { return this->size() == 0; }

         constexpr position_type get_position() const noexcept { return this->_position; }
         constexpr size_t get_bitpos()   const noexcept { return this->_position.in_bits(); }
         constexpr size_t get_bitshift() const noexcept { return this->_position.bits; }
         constexpr size_t get_bytepos()  const noexcept { return this->_position.bytes; }
         constexpr size_t get_bytespan() const noexcept { return this->_position.bytespan(); }

         constexpr header_type& header() noexcept { return this->_header; }
         constexpr const header_type& header() const noexcept { return this->_header; }

         constexpr bool is_byte_aligned() const noexcept { return this->get_bitshift() == 0; }

         constexpr void enlarge_by(size_t bytes) { this->resize(this->size() + bytes); }
         constexpr void reserve(size_t bytes);
         constexpr void resize(size_t bytes);

         constexpr void skip_bits(size_t b);
         constexpr void skip_bytes(size_t b) { this->skip_bits(b * 8); }

      protected:
         template<typename T>
         static constexpr const bool requires_checked_stream = bitstreamable_container<T>;

         template<typename T>
         constexpr void _checked_stream_if_necessary(const T& v) {
            if constexpr (!requires_checked_stream<T>) {
               this->unchecked_stream(v);
            } else {
               this->stream(v);
            }
         }

      public:
         #pragma region Stream overloads
         template<typename... Types>
         requires (sizeof...(Types) > 1 || (!requires_checked_stream<Types> && ...)) // Guard against infinite recursion, when passing one argument that requires a checked stream.
         constexpr void stream(const Types&... args) {
            (this->_checked_stream_if_necessary(args), ...);
         }

         // Multi-stream call. You can pass references to multiple fields, and stream all of them 
         // using a single call. Additionally, you can wrap any field in a stream_with_bitcount 
         // object in order to override the bitcount used.
         template<bitstreamable_primitive... Types> requires (sizeof...(Types) > 1)
         constexpr void unchecked_stream(const Types&... args) {
            (this->unchecked_stream(args), ...);
         }
         
         template<bitstreamable T>
         constexpr void unchecked_stream(const T& v) {
            v.stream(*this);
         }

         template<bitstreamable_primitive T>
         constexpr void unchecked_stream(const T& v);

         template<impl::_stream_with_bitcount::writable_specialization Wrapper>
         requires (bitstreamable_primitive<typename Wrapper::value_type> && !std::is_floating_point_v<typename Wrapper::value_type>)
         constexpr void unchecked_stream(Wrapper& v);

         template<size_t length_bitcount, bitstreamable_container T> requires (!std::is_same_v<T, QString>)
         constexpr void stream(const T& v);

         constexpr void unchecked_stream(const cobb::eight_cc& v);
         #pragma endregion

         template<bitstreamable_primitive T>
         constexpr void stream_bits(size_t bitcount, const T& v) {
            this->unchecked_stream_bits(bitcount, v);
         }
         //
         template<bitstreamable_primitive T>
         constexpr void unchecked_stream_bits(size_t bitcount, const T& v);
   };
}

#include "./writer.inl"