#pragma once
#include <cassert>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "helpers/type_traits.h"
#include "detailed_notice.h"
#include "file.h"
#include "blocks/_factory.h"
#include "types/Float16.h"

namespace nifDK {
   class file;

   struct Float16;
   struct NiMatrix33;
   struct NiTransform;

   namespace impl::file_reader {
      template<typename T> concept IsLiteral = requires {
         requires (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>);
      };
      template<typename T> concept IsLiteralIsh = IsLiteral<T> || (std::is_bounded_array_v<T> && IsLiteral<std::remove_extent_t<T>>);
   }

   class file_reader {
      public:
         class read_error : public std::runtime_error {
            public:
               read_error(notice_code_t c) : std::runtime_error("NIF-reading error"), code(c) {};

               const notice_code_t code = default_notice_code;
         };

      protected:
         const void* _data     = nullptr;
         size_t      _size     = 0;
         size_t      _position = 0;
         //
         inline const void* _at() const noexcept { return (const void*)((std::intptr_t)this->_data + this->_position); }

         file* subject = nullptr;
         struct {
            int32_t current_block = -1;
            size_t  block_offset  = 0;
         } state;
         detailed_notice error;

         bool _read_ref(void*&);
         void _on_read_failure();

         inline void _require_size(size_t s) {
            if (!this->is_in_bounds(s))
               this->_on_read_failure();
         }

         template<size_t S> inline void _read(auto& out);
         template<typename T, size_t S = sizeof(T)> inline void _read<S>(T& out) {
            if (!this->is_in_bounds(S))
               this->_on_read_failure();
            this->unchecked_read(out);
         }
         template<typename T, size_t S> inline void _read(T& out);
         template<typename T, size_t S, typename V> inline void _read<T, S>(V& out) {
            if (!this->is_in_bounds(S * sizeof(T)))
               this->_on_read_failure();
            this->unchecked_read(out);
         }

      public:
         file_reader(file* o, const void* d, size_t s) : _data(d), _size(s), subject(o) {}

         inline const void* data() const noexcept { return this->_data; }
         inline size_t size() const noexcept { return this->_size; }
         inline size_t position() const noexcept { return this->_position; }
         inline bool empty() const noexcept { return this->_data == nullptr || this->_size == 0; }

         inline file_version version() const noexcept { return this->subject->header.version; }

         inline const void* data_at(size_t p) const noexcept {
            if (p > this->_size)
               return nullptr;
            if (this->_data == nullptr)
               return nullptr;
            return (const void*)((std::intptr_t)this->_data + p);
         }

         inline bool at_end() const noexcept { return this->_position == this->_size; }
         inline bool is_in_bounds(size_t s) const noexcept { return this->_position + s <= this->_size; }

         inline void skip(size_t s) {
            this->_position += s;
            if (this->_position >= this->_size)
               this->_position = this->_size;
         }
         void require_size(size_t s) {
            this->_require_size(s);
         }

         #pragma region read
         inline void read(void* buffer, size_t size) {
            this->_require_size(size);
            this->unchecked_read(buffer, size);
         }
         template<typename T> requires (impl::file_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline void read(T& field) {
            if constexpr (cobb::is_std_array<T>) {
               constexpr size_t total_size = sizeof(T::value_type) * field.size();
               this->_require_size(total_size);
               this->unchecked_read(&field, total_size);
            } else {
               this->read(&field, sizeof(T));
            }
         }

         template<typename Desired> bool read_ref(Desired*& out) {
            out = nullptr;
            //
            void* instance;
            if (!this->_read_ref(instance))
               return false;
            auto* casted = dynamic_cast<Desired>(instance);
            if (!casted)
               return false;
            out = casted;
            return true;
         }

         void read_line_string(std::string& out);

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

         template<typename T> void read_vector_contents(std::vector<T>& out) {
            auto size = out.size() * sizeof(T);
            this->_require_size(size);
            this->unchecked_read(out.data(), size);
         }
         template<int N, typename F, glm::qualifier Q> void read_vector_contents<glm::vec<N, F, Q>>(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size() * (sizeof(F) * N);
            this->_require_size(size);
            this->unchecked_read(out.data(), size);
         }
         
         template<typename T> void read_half_vector_contents(std::vector<T>&);
         template<int N, typename F, glm::qualifier Q> void read_half_vector_contents<glm::vec<N, F, Q>>(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size();
            //
            std::vector<uint16_t> halves(size);
            this->require_size(size * sizeof(uint16_t));
            this->unchecked_read(halves.data(), size * sizeof(uint16_t));
            //
            for (size_t i = 0; i < size; ++i) {
               out[i / N][i % N] = Float16(halves[i]);
            }
         }
         #pragma endregion
         #pragma region unchecked_read
         inline bool unchecked_read(void* buffer, size_t size) {
            memcpy(buffer, _at(), size);
            this->_position += size;
         }
         template<typename T> requires (impl::file_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline void unchecked_read(T& field) {
            if constexpr (cobb::is_std_array<T>) {
               constexpr size_t total_size = sizeof(T::value_type) * field.size();
               return this->unchecked_read(&field, total_size);
            } else {
               return this->unchecked_read(&field, sizeof(T));
            }
         }

         template<typename T> void unchecked_read_vector_contents(std::vector<T>& out) {
            auto size = out.size() * sizeof(T);
            this->unchecked_read(out.data(), size);
         }
         template<int N, typename F, glm::qualifier Q> void unchecked_read_vector_contents<glm::vec<N, F, Q>>(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size() * (sizeof(F) * N);
            this->unchecked_read(out.data(), size);
         }
         
         template<typename T> void unchecked_read_half_vector_contents(std::vector<T>&);
         template<int N, typename F, glm::qualifier Q> void unchecked_read_half_vector_contents<glm::vec<N, F, Q>>(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size();
            //
            std::vector<uint16_t> halves(size);
            this->unchecked_read(halves.data(), size * sizeof(uint16_t));
            //
            for (size_t i = 0; i < size; ++i) {
               out[i / N][i % N] = Float16(halves[i]);
            }
         }
         #pragma endregion

         #pragma region reading types
         inline void read(file_version& v) { this->_read<4>(v); }
         void unchecked_read(file_version&);
         //
         inline void read(Float16& v) { this->_read<2>(v); }
         void unchecked_read(Float16&);
         //
         inline void read(NiMatrix33& v) { this->_read<float, 9>(v); }
         void unchecked_read(NiMatrix33&);
         inline void read(NiTransform& v) { this->_read<float, 3 + 9 + 1>(v); }
         void unchecked_read(NiTransform&);
         #pragma endregion

         void raise_error(const detailed_notice&);
         void raise_error(notice_code_t);
         //
         template<typename T> requires requires(file_reader& f, T x) { { f.raise_error(x) }; }
         void throw_error(T x) {
            this->raise_error(x);
            throw read_error(this->error.code);
         }
   };
}