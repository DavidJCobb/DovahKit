#pragma once
#if _DEBUG
   #include <cassert>
#endif
#include <cstddef> // std::uintptr_t
#include <cstdio> // FILE*
#include <filesystem>
#include <system_error> // std::error_code

namespace lu::files {
   // Read-only memory-mapped file.
   class mapped_file {
      public:
         using size_type = size_t;

      protected:
         void*     _view = nullptr;
         size_type _size = 0;

      public:
         mapped_file() noexcept {}
         mapped_file(const std::filesystem::path& path) { this->open(path); }
         ~mapped_file();

         #pragma region Copy/move
            mapped_file(const mapped_file&) = delete;
            mapped_file& operator=(const mapped_file&) = delete;

            mapped_file(mapped_file&&) noexcept;
            mapped_file& operator=(mapped_file&&) noexcept;
         #pragma endregion
         
         constexpr const void* data() const noexcept { return this->_view; }
         constexpr const void* data_at(std::ptrdiff_t offset) const noexcept {
            return (const void*)((std::uintptr_t)this->_view + offset);
         }
         constexpr size_type size() const noexcept { return this->_size; }

         constexpr operator bool() const noexcept { return this->_view != nullptr; }
         
         void open(FILE*);
         void open(FILE*, std::error_code&) noexcept;
         void open(const std::filesystem::path&);
         void open(const std::filesystem::path&, std::error_code&) noexcept;
         
         void read_from(size_type pos, void* dst_buffer, size_type dst_size) const noexcept {
            #if _DEBUG
               assert(!!this->_view);
               assert(this->is_in_bounds(pos, dst_size));
            #endif
            std::memcpy(dst_buffer, this->data_at(pos), dst_size);
         }

         template<typename T>
         void read_from(size_type pos, T& field) const noexcept {
            this->read_from(pos, &field, sizeof(field));
         }
         
         constexpr bool is_in_bounds(size_type offset, size_type bytes) const noexcept {
            return offset + bytes <= this->_size;
         }

      protected:
         void _open_from_native_handle(void* handle, std::error_code&) noexcept;
         void _close() noexcept;
   };
}