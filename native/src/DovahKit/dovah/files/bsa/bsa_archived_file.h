#pragma once
#include <string>
#include "../../../helpers/memory.h"

namespace dovah {
   class bsa_archive;
   class bsa_load_order;

   class bsa_archived_file {
      friend class bsa_archive;
      friend class bsa_load_order;
      public:
         enum class error_code {
            none,
            lz4_error,
         };
      protected:
         cobb::generic_buffer owned; // used for files that needed to be decompressed and, theoretically, loose files
         struct {
            const void* data = nullptr;
            size_t size = 0;
         } shared; // used for files that could be pulled directly out of a memory-mapped BSA file
         error_code error = error_code::none;
         //
      public:
         bsa_archived_file() {};
         //
         bsa_archived_file(const cobb::generic_buffer& buf) : owned(buf) {};
         bsa_archived_file(cobb::generic_buffer&& buf) : owned(buf) {};
         //
         bsa_archived_file(const void* shared_data, size_t s) {
            this->shared.data = shared_data;
            this->shared.size = s;
         }
         
         inline const void* data() const noexcept {
            const void* d = this->owned.data();
            if (!d)
               d = this->shared.data;
            return d;
         }
         inline uint64_t size() const noexcept {
            if (this->owned.data())
               return this->owned.size();
            return this->shared.size;
         }
         inline error_code get_error() const noexcept { return this->error; }
         inline bool has_error() const noexcept { return this->error != error_code::none; }
         inline bool is_shared() const noexcept { return !this->owned.data(); }
         
         void read(size_t offset, void* value, size_t size) const noexcept {
            if (!this->data())
               return;
            memcpy(value, (const uint8_t*)this->data() + offset, size);
         }
         template<typename T> void read(size_t offset, T& value) const noexcept {
            if (!this->data()) {
               value = T();
               return;
            }
            memcpy(&value, (const uint8_t*)this->data() + offset, sizeof(T));
         }
         template<> void read(size_t offset, std::string& value) const noexcept = delete;
         template<> void read(size_t offset, std::wstring& value) const noexcept = delete;
   };
}