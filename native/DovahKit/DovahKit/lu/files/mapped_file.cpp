#include "./mapped_file.h"
#include <cstdint>
#include <windows.h> // CloseHandle, CreateFile(A|W), GetLastError
#include <io.h> // _get_osfhandle
#include <memoryapi.h> // MapViewOfFile, UnmapViewOfFile

namespace lu::files {
   mapped_file::~mapped_file() {
      this->_close();
   }
   
   #pragma region Copy/move
      mapped_file::mapped_file(mapped_file&& other) noexcept {
         this->_view = other._view;
         this->_size = other._size;
         other._view = nullptr;
         other._size = 0;
      }
      mapped_file& mapped_file::operator=(mapped_file&& other) noexcept {
         this->_view = other._view;
         this->_size = other._size;
         other._view = nullptr;
         other._size = 0;
         return *this;
      }
   #pragma endregion

   void mapped_file::open(FILE* file) {
      std::error_code ec;
      this->open(file, ec);
      if (ec)
         throw std::system_error(ec);
   }
   void mapped_file::open(FILE* file, std::error_code& ec) noexcept {
      this->_close();
      uint64_t size = 0;
      {
         fpos_t _pos;
         fgetpos(file, &_pos);
         fseek(file, 0, SEEK_END);
         fpos_t _end;
         fgetpos(file, &_end);
         size = _end;
         fseek(file, _pos, SEEK_SET);
      }
      this->_size = size;
      this->_open_from_native_handle((HANDLE)_get_osfhandle(_fileno(file)), ec);
   }
   void mapped_file::open(const std::filesystem::path& path) {
      std::error_code ec;
      this->open(path, ec);
      if (ec)
         throw std::system_error(ec);
   }
   void mapped_file::open(const std::filesystem::path& path, std::error_code& ec) noexcept {
      this->_close();
      HANDLE file = CreateFile(path.native().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (file == INVALID_HANDLE_VALUE) {
         ec = std::error_code(GetLastError(), std::system_category());
         return;
      }
      this->_size = std::filesystem::file_size(path, ec);
      if (ec) {
         CloseHandle(file);
         return;
      }
      this->_open_from_native_handle(file, ec);
      if (ec)
         CloseHandle(file);
   }

   void mapped_file::_open_from_native_handle(void* handle, std::error_code& ec) noexcept {
      HANDLE mapped_file = CreateFileMappingW((HANDLE)handle, NULL, PAGE_READONLY, (uint32_t)(this->_size >> 0x20), (uint32_t)this->_size, NULL);
      if (mapped_file == NULL) {
         ec = std::error_code(GetLastError(), std::system_category());
         return;
      }
      this->_view = MapViewOfFile(mapped_file, FILE_MAP_READ, 0, 0, 0);
      CloseHandle(mapped_file);
      if (this->_view == nullptr) {
         ec = std::error_code(GetLastError(), std::system_category());
      } else {
         ec = {};
      }
   }
   void mapped_file::_close() noexcept {
      if (this->_view) {
         UnmapViewOfFile(this->_view);
         this->_view = nullptr;
      }
   }
}