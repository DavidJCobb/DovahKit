#pragma once
#include <cstdint>
#include <string>
#include "../../helpers/memory.h"
#include "../../core.h"
#include "../common.h"

namespace dovah {
   class  form_stub;
   struct localized_string;

   namespace tes_file_writing {
      class file_writer;
      class record;
      class subrecord;

      class group {
         friend class file_writer;
         public:
            using type     = dovah::tes_file_group_type;
            using header_t = dovah::tes_file_group_header;
            //
         protected:
            header_t header;
            uint32_t pos = 0; // position of the start of the group, i.e. just before the signature
            //
         public:
            inline operator bool() const noexcept { return this->exists(); }
            inline bool exists() const noexcept { return this->header.signature != 0; }
            //
            group* get_parent() const noexcept;
      };

      class record {
         friend class file_writer;
         friend class subrecord;
         public:
            using header_t = dovah::tes_file_record_header;
         protected:
            record(file_writer& o) : owner(o) {}
            //
            file_writer& owner;
            header_t header;
            uint32_t pos = 0;
            cobb::generic_buffer data; // record body (uncompressed)
            //
            void _write_impl(const void* source, uint32_t size);
            void _write_impl(const tes_file_subrecord_header&);
            template<typename T> inline void _write(const T& v) {
               this->_write_impl(&v, sizeof(T));
            }
            template<> inline void _write(const std::string& v) {
               this->_write_impl(v.c_str(), v.size() + 1);
            }
            template<> inline void _write(const cobb::generic_buffer& v) {
               this->_write_impl(v.data(), v.size());
            }
            template<> inline void _write(const tes_file_group_header& v) = delete;
            template<> inline void _write(const tes_file_record_header& v) = delete;
            template<> inline void _write(const tes_file_subrecord_header& v) {
               this->_write_impl(v);
            }
            //
            void _close_current_subrecord();
            //
         public:
            record& operator=(const record& other) = delete; // no copy
            record(record& other) = delete; // no copy
            //
            inline bool is_skyrim_special() const noexcept { return this->header.version >= 44; }
            //
            operator bool() const noexcept { return this->exists(); }
            inline bool exists() const noexcept { return this->header.signature != 0; }
            //
            subrecord& get_current_subrecord() const noexcept;
            subrecord& open_next_subrecord(uint32_t signature);
            void close();
            //
            inline void reserve_more(uint32_t bytes) { this->data.reserve(this->pos + bytes); }
            //
            void write_string_subrecord(uint32_t signature, const char* s);
            void write_string_subrecord(uint32_t signature, const std::string& s);
      };

      class subrecord {
         friend class file_writer;
         friend class record;
         public:
            using header_t = dovah::tes_file_subrecord_header;
         protected:
            subrecord(file_writer& o) : owner(o) {}
            //
            file_writer& owner;
            header_t header;
            uint32_t pos = 0;
            cobb::generic_buffer data; // record body (uncompressed)
            //
         public:
            subrecord& operator=(const subrecord& other) = delete; // no copy
            subrecord(subrecord& other) = delete; // no copy
            //
            inline operator bool() const { return this->header.signature != 0; }
            inline bool exists() const noexcept { return this->header.signature != 0; }
            //
            record& get_containing_record() const;
            inline bool is_skyrim_special() const noexcept { return this->get_containing_record().is_skyrim_special(); }
            //
            inline void reserve_more(uint32_t bytes) { this->data.reserve(this->pos + bytes); }
            //
            #pragma region writing
            //
            // NOTE: Prefer record::write_string_subrecord for writing subrecords that consist 
            // entirely of strings. Always use it for const char*.
            //
            void write(const void* source, uint32_t size);
            template<typename T> inline void write(const T& v) {
               this->write(&v, sizeof(T));
            }
            template<> inline void write(const std::string& v) {
               this->write(v.c_str(), v.size() + 1);
            }
            template<> inline void write(const cobb::generic_buffer& v) {
               this->write(v.data(), v.size());
            }
            template<> inline void write(const tes_file_group_header& v) = delete;
            template<> inline void write(const tes_file_record_header& v) = delete;
            template<> inline void write(const tes_file_subrecord_header& v) = delete;
            #pragma endregion
            //
            void close();
      };
   }
}