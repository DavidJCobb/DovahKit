#pragma once
#include <bit>
#include <cassert>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "helpers/byteswap.h"
#include "helpers/endian.h"
#include "helpers/passkey.h"
#include "helpers/type_traits/is_std_array.h"
#include "helpers/type_traits/is_std_vector.h"
#include "helpers/unreachable.h"
#include "helpers/glm/type_traits.h"
#include "detailed_notice.h"
#include "file.h"
#include "blocks/_factory.h"
#include "blocks/unknown.h"
#include "types/Float16.h"

namespace nifDK {
   class  file;
   class  file_reader;
   struct file_version;

   namespace impl::file_reader {
      template<typename T> concept IsLiteral = requires {
         requires (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>);
      };
      template<typename T> concept IsLiteralIsh = IsLiteral<T> || (std::is_bounded_array_v<T> && IsLiteral<std::remove_extent_t<T>>);

      template<typename T> concept OffersReaderHook = requires(T& x, ::nifDK::file_reader& fr) { { x.read(fr) }; };
      template<typename T> concept OffersUncheckedReaderHook = requires(T & x, ::nifDK::file_reader & fr) { { x.unchecked_read(fr) }; };

      template<bool checked, typename T> concept OffersHook = (checked ? OffersReaderHook<T> : OffersUncheckedReaderHook<T>);

      template<bool checked, typename T> concept _AllowSimpleCallList = requires {
         requires cobb::is_std_array<T> || cobb::is_std_vector<T>;
         typename T::value_type;
         requires IsLiteralIsh<typename T::value_type> || OffersHook<checked, typename T::value_type> || cobb::glm::is_vec<typename T::value_type> || cobb::glm::is_mat<typename T::value_type> || cobb::glm::is_quat<typename T::value_type>;
      };
      template<bool checked, typename T> concept AllowSimpleCall = IsLiteralIsh<T> || _AllowSimpleCallList<checked, T> || OffersHook<checked, T> || cobb::glm::is_vec<T> || cobb::glm::is_mat<T> || cobb::glm::is_quat<T>;
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
         std::endian endianness = std::endian::native;
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

         bool _read_ref(block*&);
         void _on_read_failure();

         inline void _require_size(size_t s) {
            if (!this->is_in_bounds(s))
               this->_on_read_failure();
         }
         template<bool checked, typename T> void _fix_endianness(T& v) {
            using namespace impl::file_reader;
            //
            if constexpr (checked ? OffersReaderHook<T> : OffersUncheckedReaderHook<T>)
               return;
            if constexpr (sizeof(T) == 1)
               return;
            if (this->endianness == std::endian::native)
               return;
            //
            if constexpr (cobb::is_std_array<T> || cobb::is_std_vector<T>) {
               for (auto& item : v)
                  _fix_endianness<checked>(item);
               return;
            }
            if constexpr (cobb::glm::is_vec<T>) {
               for (int i = 0; i < cobb::glm::vec_length<T>; ++i)
                  v[i] = cobb::byteswap(v[i]);
               return;
            }
            if constexpr (cobb::glm::is_mat<T>) {
               using traits = cobb::glm::mat_traits<T>;
               for (int i = 0; i < traits::cols; ++i)
                  for(int j = 0; j < traits::rows; ++j)
                     v[i][j] = cobb::byteswap(v[i][j]);
               return;
            }
            if constexpr (cobb::glm::is_quat<T>) {
               v.x = cobb::byteswap(v.x);
               v.y = cobb::byteswap(v.y);
               v.z = cobb::byteswap(v.z);
               v.w = cobb::byteswap(v.w);
               return;
            }
            if constexpr (IsLiteralIsh<T>) {
               if constexpr (std::is_bounded_array_v<T>) {
                  for (size_t i = 0; i < std::extent<T>::value; ++i)
                     v[i] = cobb::byteswap(v[i]);
               } else {
                  v = cobb::byteswap(v);
               }
               return;
            }
         }
         template<bool checked, typename T> void _read_field(T& v) {
            using namespace impl::file_reader;
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
            if constexpr (cobb::is_std_array<T> || cobb::is_std_vector<T>) {
               //
               // We handle arrays and vectors the same way; we assume that vectors have already been 
               // expanded to the appropriate size, and we load the items inside. The reason we do this, 
               // rather than offering a function to load the vector's size and handle that, is because 
               // there are several cases where a block may have multiple vectors who share a single size 
               // value (i.e. struct-of-arrays).
               //
               using V = T::value_type;
               constexpr bool can_read_in_bulk = ([]() {
                  if constexpr (cobb::glm::is_vec<V>) {
                     return cobb::glm::vec_traits<V>::is_contiguous;
                  }
                  if constexpr (cobb::glm::is_mat<V>) {
                     return cobb::glm::mat_traits<V>::is_contiguous;
                  }
                  if constexpr (cobb::glm::is_quat<V>) {
                     return cobb::glm::quat_traits<V>::is_contiguous;
                  }
                  if constexpr (OffersHook<checked, V>) {
                     return false;
                  }
                  return true;
               })();
               if constexpr (checked) {
                  this->_require_size(sizeof(V) * v.size());
               }
               if constexpr (can_read_in_bulk) {
                  this->unchecked_read(v.data(), sizeof(V) * v.size());
               } else {
                  if constexpr (OffersHook<checked, V>) {
                     //
                     // The item type relies on a hook (i.e. it supplies its own read/unchecked_read functions), 
                     // so we have to make sure we use the right one by passing the "checked" template parameter.
                     //
                     for (auto& item : v)
                        this->_read_field<checked>(item);
                  } else {
                     //
                     // We already checked the item's size.
                     //
                     for (auto& item : v)
                        this->_read_field<false>(item);
                  }
                  //
                  // Calling _read_field per item will do an endianness fixup per item, so we can and must return 
                  // here to skip the endianness fixup for the list as a whole.
                  //
                  return;
               }
               // ...and fall through to endianness check.
            } else if constexpr (cobb::glm::is_vec<T>) {
               //
               // We use some compile-time programming here to make sure we handle GLM structs properly -- 
               // checking whether the size of the struct is the same as the combined sizes of its members 
               // (i.e. no padding).
               //
               constexpr auto axes = cobb::glm::vec_length<T>;
               constexpr auto size = sizeof(T::value_type) * axes;
               if constexpr (checked) {
                  this->_require_size(size);
               }
               if constexpr (sizeof(T) == size) {
                  this->unchecked_read(&v, size);
               } else {
                  for (int i = 0; i < axes; ++i)
                     this->unchecked_read(v[i]);
               }
               // ...and fall through to endianness check.
            } else if constexpr (cobb::glm::is_mat<T>) {
               using traits = cobb::glm::mat_traits<T>;
               constexpr auto count = traits::size;
               constexpr auto bytes = sizeof(T::value_type) * count;
               if constexpr (checked) {
                  this->_require_size(bytes);
               }
               if constexpr (sizeof(T) == bytes) {
                  this->unchecked_read(&v, bytes);
               } else {
                  for (int i = 0; i < traits::cols; ++i)
                     for (int j = 0; j < traits::rows; ++j)
                        this->unchecked_read(v[i][j]);
               }
               // ...and fall through to endianness check.
            } else if constexpr (cobb::glm::is_quat<T>) {
               using traits = cobb::glm::quat_traits<T>;
               constexpr auto bytes = sizeof(T::value_type) * traits::size;
               if constexpr (checked) {
                  this->_require_size(bytes);
               }
               if constexpr (sizeof(T) == bytes) {
                  this->unchecked_read(&v, bytes);
               } else {
                  this->unchecked_read(v.x);
                  this->unchecked_read(v.y);
                  this->unchecked_read(v.y);
                  this->unchecked_read(v.w);
               }
               // ...and fall through to endianness check.
            } else if constexpr (IsLiteralIsh<T>) {
               if constexpr (checked) {
                  this->_require_size(sizeof(T));
               }
               this->unchecked_read(&v, sizeof(T));
               // ...and fall through to endianness check.
            }
            this->_fix_endianness<checked>(v);
         }

         template<bool checked, int N, typename F, glm::qualifier Q> void _read_half_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
            auto size = out.size();
            //
            if constexpr (checked) {
               this->_require_size(size * sizeof(uint16_t));
            }
            std::vector<Float16> halves(size);
            this->_read_field<checked>(halves);
            for (size_t i = 0; i < size; ++i) {
               out[i / N][i % N] = halves[i];
            }
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
            auto& uv = this->subject->header.user_versions;
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

         void read_endianness(file_passkey);
         void read_string_table(file_passkey);
         inline block_guard enter_block(file_passkey, int32_t bi, size_t size, const std::string& block_type) {
            return block_guard(*this, bi, size, block_type);
         }

         int32_t index_of_block(block*) const;

         inline const void* data_at(size_t p) const noexcept {
            if (p > this->states.current.size)
               return nullptr;
            if (this->states.current.data == nullptr)
               return nullptr;
            return (const void*)((std::intptr_t)this->data() + p);
         }

         inline bool at_end() const noexcept { return this->position() == this->size(); }
         inline bool is_in_bounds(size_t s) const noexcept { return this->position() + s <= this->size(); }
         inline const detailed_notice& error_details() const noexcept { return this->error; }

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
         template<typename T> requires impl::file_reader::AllowSimpleCall<true, T> inline void read(T& field) {
            this->_read_field<true>(field);
         }

         // overload to require a specific endianness, overriding the one specified in the file header
         template<std::endian E, typename T> requires impl::file_reader::IsLiteralIsh<T> inline void read(T& field) {
            this->_require_size(sizeof(T));
            this->unchecked_read<E>(field);
         }

         template<typename Desired> requires std::is_polymorphic_v<Desired>
         bool read_ref(Desired*& out) {
            out = nullptr;
            //
            block* instance;
            if (!this->_read_ref(instance))
               return false;
            if (!instance)
               return true;
            //
            // Non-null reference. Validate the pointer's type.
            //
            Desired* casted = dynamic_cast<Desired*>(instance);
            if (!casted) {
               const auto& ref = *instance;
               if (typeid(ref) == typeid(block_types::unknown_block)) {
                  //
                  // Mismatches are not an error (though we DO NOT WRITE THE POINTER) if the 
                  // target block is an unknown block type. This ensures that if some block 
                  // type A can refer to B, C, D, E, and F, we can at least implement base-
                  // line support for A without immediately being obligated to implement 
                  // full support for B through F.
                  //
                  return true;
               }
               return false;
            }
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
            this->_read_field<true>(out);
         }
         template<int N, typename F, glm::qualifier Q> void read_half_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
            this->_read_half_vector_contents<true>(out);
         }
         #pragma endregion
         #pragma region unchecked_read
         inline void unchecked_read(void* buffer, size_t size) {
            memcpy(buffer, _at(), size);
            this->states.current.position += size;
         }
         template<typename T> requires impl::file_reader::AllowSimpleCall<false, T> inline void unchecked_read(T& field) {
            this->_read_field<false>(field);
         }

         // overload to require a specific endianness, overriding the one specified in the file header
         template<std::endian E, typename T> requires impl::file_reader::IsLiteralIsh<T> inline void unchecked_read(T& field) {
            if constexpr (sizeof(T) == 1 || E == std::endian::native) {
               return this->unchecked_read(field);
            }
            this->unchecked_read(field);
            field = cobb::endian_cast<E>(field);
         }

         template<typename T> void unchecked_read_vector_contents(std::vector<T>& out) {
            this->_read_field<false>(out);
         }
         template<int N, typename F, glm::qualifier Q> void unchecked_read_half_vector_contents(std::vector<glm::vec<N, F, Q>>& out) {
            this->_read_half_vector_contents<false>(out);
         }
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