#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>
#include "./exceptions/write_exception.h"
#include "./_base.h"
#include "./bitstreamable_struct.h"
#include "./bitstreamable_container.h"
#include "./bitstreamable_primitive.h"
#include "./data_header.h"
#include "./override_bitcount.h"
#include "./position.h"

// streamable types:
#include "../eight_cc.h"

class QString;
#ifdef QT_CORE_LIB
   #include <string>
   #include <QString>
#endif

namespace cobb::bitstreams {
   class writer : public _bitstream_base {
      public:
         using header_type   = data_header;
         using position_type = typename position;

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
         template<typename Exception, typename... Types> requires std::is_base_of_v<exceptions::write_exception, Exception>
         [[noreturn]] constexpr void _throw_exception(Types&&... args) const {
            auto except = Exception(std::forward<Types>(args)...);
            except.where_were_we = this->_where_are_we;
            throw except;
         }

      public:
         #pragma region Stream overloads
         // Multi-stream call. You can pass references to multiple fields, and stream all of them 
         // using a single call. Additionally, you can use override_bitcount to wrap any field and 
         // override its bitcount.
         template<typename... Types> requires (sizeof...(Types) > 1)
         constexpr void stream(const Types&... args) {
            (this->stream(args), ...);
         }
         
         template<bitstreamable_struct T>
         constexpr void stream(const T& v) {
            const auto step = add_parse_step_for_typename<T>();
            v.stream(*this);
         }

         template<bitstreamable_primitive T>
         constexpr void stream(const T& v);

         template<impl::_override_bitcount::writable_specialization Wrapper>
         constexpr void stream(const Wrapper& v);

         template<size_t length_bitcount, bitstreamable_container T> requires (!std::is_same_v<T, QString>)
         constexpr void stream(const T& v);

         constexpr void stream(const cobb::eight_cc& v);
         #pragma endregion

         template<bitstreamable_primitive T>
         constexpr void stream_bits(size_t bitcount, const T& v);
         
         #ifdef QT_CORE_LIB
            template<size_t length_bitcount>
            void stream(const QString& v) {
               auto utf8 = v.toUtf8();
               auto sv   = std::string_view(utf8.constData(), utf8.size());

               this->stream<length_bitcount>(sv);
            }
         #endif
   };
}

#include "./writer.inl"