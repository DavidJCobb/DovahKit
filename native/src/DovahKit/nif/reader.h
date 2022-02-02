#pragma once
#include <cassert>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <glm/glm.hpp>
#include "helpers/passkey.h"
#include "helpers/type_traits.h"
#include "helpers/unreachable.h"
#include "detailed_notice.h"
#include "file.h"
#include "blocks/_factory.h"
#include "types/Float16.h"
#include "types/NiMatrix33.h" // this is a using declaration; can't forward-declare those

namespace nifDK {
   class  file;
   struct file_version;

   struct Float16;
   struct NiBound;
   struct NiColor;
   struct NiColorA;
   //struct NiMatrix33; // this is a using declaration; can't forward-declare those
   struct NiTransform;

   namespace impl::file_reader {
      template<typename T> concept IsLiteral = requires {
         requires (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>);
      };
      template<typename T> concept IsLiteralIsh = IsLiteral<T> || (std::is_bounded_array_v<T> && IsLiteral<std::remove_extent_t<T>>);
   }

   class file_reader {
      public:
         using file_passkey = cobb::passkey<file, file_reader>;

         class read_error : public std::runtime_error {
            public:
               read_error(notice_code_t c) : std::runtime_error("NIF-reading error"), code(c) {};

               const notice_code_t code = default_notice_code;
         };

      protected:
         struct state {
            const void* data     = nullptr;
            size_t      size     = 0;
            size_t      position = 0;
         };
         //
         struct {
            state current;
            state backup;
         } states;
         int32_t     _block_index = -1;
         std::string _block_type;
         struct {
            uint32_t max_length = 0;
            std::vector<std::string> list;
         } string_table;
         //
         inline const void* _at() const noexcept { return (const void*)((std::intptr_t)this->data() + this->position()); }

         file* subject = nullptr;
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

         // using this instead of start/end functions makes it exception-safe
         struct block_guard {
            friend class file_reader;
            protected:
               block_guard(file_reader&, int32_t block_index, size_t block_size, const std::string& block_type);

               file_reader& owner;
               int32_t      block_index;

            public:
               ~block_guard();
         };

      public:
         file_reader(file* o, const void* d, size_t s) : subject(o) {
            this->states.current = { d, s, 0 };
         }

         inline const void* data() const noexcept { return this->states.current.data; }
         inline size_t size() const noexcept { return this->states.current.size; }
         inline size_t position() const noexcept { return this->states.current.position; }
         inline bool empty() const noexcept { return this->states.current.data == nullptr || this->states.current.size == 0; }

         inline file_version version() const noexcept { return this->subject->header.version; }
         template<size_t N> requires (N == 1 || N == 2) inline uint32_t user_version() const noexcept {
            auto& uv = this->subject.header.user_versions;
            if constexpr (N == 1)
               return uv.primary;
            if constexpr (N == 2)
               return uv.secondary;
            cobb::unreachable();
         }

         inline const std::string& block_type() const noexcept { return this->_block_type; }
         inline int32_t block_index() const noexcept { return this->_block_index; }
         inline bool is_in_block() const noexcept { return this->block_index() != -1; }
         inline size_t file_position() const {
            if (this->is_in_block())
               return this->states.current.position + this->states.backup.position;
            return this->position();
         }

         void read_string_table(file_passkey);
         inline block_guard enter_block(file_passkey, int32_t bi, size_t size, const std::string& block_type) {
            return block_guard(*this, bi, size, block_type);
         }

         inline const void* data_at(size_t p) const noexcept {
            if (p > this->states.current.size)
               return nullptr;
            if (this->states.current.data == nullptr)
               return nullptr;
            return (const void*)((std::intptr_t)this->data() + p);
         }

         inline bool at_end() const noexcept { return this->position() == this->size(); }
         inline bool is_in_bounds(size_t s) const noexcept { return this->position() + s <= this->size(); }

         inline void skip(size_t s) {
            auto& p = this->states.current.position;
            p += s;
            if (p >= this->size())
               p = this->size();
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

         template<int N, typename T, glm::qualifier Q> inline void read(glm::vec<N, T, Q>& out) {
            this->_require_size(sizeof(T) * N);
            this->unchecked_read(out);
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

         void read_indexed_string(std::string&);
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
         template<int N, typename F, glm::qualifier Q> void read_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size() * (sizeof(F) * N);
            this->_require_size(size);
            this->unchecked_read(out.data(), size);
         }
         
         template<int N, typename F, glm::qualifier Q> void read_half_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
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
            this->states.current.position += size;
         }
         template<typename T> requires (impl::file_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline void unchecked_read(T& field) {
            if constexpr (cobb::is_std_array<T>) {
               constexpr size_t total_size = sizeof(T::value_type) * field.size();
               return this->unchecked_read(&field, total_size);
            } else {
               return this->unchecked_read(&field, sizeof(T));
            }
         }

         template<int N, typename T, glm::qualifier Q> inline void unchecked_read(glm::vec<N, T, Q>& out) {
            if constexpr (sizeof(glm::vec<N, T, Q>) == sizeof(T) * N) {
               this->unchecked_read(&out, sizeof(T) * N);
            } else {
               for (int i = 0; i < N; ++i)
                  this->unchecked_read(&out[i], sizeof(T));
            }
         }

         template<typename T> void unchecked_read_vector_contents(std::vector<T>& out) {
            auto size = out.size() * sizeof(T);
            this->unchecked_read(out.data(), size);
         }
         template<int N, typename F, glm::qualifier Q> void unchecked_read_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size() * (sizeof(F) * N);
            this->unchecked_read(out.data(), size);
         }
         
         template<int N, typename F, glm::qualifier Q> void unchecked_read_half_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
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
         inline void read(NiBound& v) { this->_read<float, 4>(v); }
         void unchecked_read(NiBound&);
         inline void read(NiColor& v) { this->_read<3>(v); }
         void unchecked_read(NiColor&);
         inline void read(NiColorA& v) { this->_read<4>(v); }
         void unchecked_read(NiColorA&);
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