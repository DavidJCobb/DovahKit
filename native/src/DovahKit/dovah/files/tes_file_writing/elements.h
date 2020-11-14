#pragma once
#include <cstdint>
#include <string>
#include "../../helpers/memory.h"
#include "../../helpers/miscellaneous.h"
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
            void _clear(); // clears a record's state without writing it to the file; file-writing internals can call this to abort writing a record
            void _close(); // writes the record to the file (closing any open subrecord) and then clears its state
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
            //
            inline void reserve_more(uint32_t bytes) { this->data.reserve(this->pos + bytes); }
            //
            void write_formID_subrecord(uint32_t signature, const form_reference_t&, bool only_if_non_empty = false);
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
            void _fixup_form_id(bare_form_id_t& id) const noexcept;
            void _write_impl(const form_reference_t&);
            void _write_impl(const struct_form_reference_t&);
            void _write_impl(const localized_string&);
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
            inline uint32_t signature() const noexcept { return this->header.signature; }
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
            template<> inline void write(const form_reference_t& field) { return this->_write_impl(field); }
            template<> inline void write(const struct_form_reference_t& field) { return this->_write_impl(field); }
            template<> inline void write(const localized_string& field) { return this->_write_impl(field); }
            template<> inline void write(const tes_file_group_header& v) = delete;
            template<> inline void write(const tes_file_record_header& v) = delete;
            template<> inline void write(const tes_file_subrecord_header& v) = delete;
            template<> inline void write(const form_id_t& v) = delete;
            //
            void write_signature(uint32_t);
            //
            template<int length_bytes> void write_length_prefixed_string(const std::string& v) {
               using int_t = cobb::bytecount_to_int_t<length_bytes>;
               int_t length = v.size();
               this->reserve_more(length_bytes + v.size() + 1);
               this->write(length);
               this->write(v);
            }
            //
            void skip_bytes(uint32_t bytes);
            #pragma endregion
            //
            void close();
      };
   }

   using tes_record_writer    = tes_file_writing::record;
   using tes_subrecord_writer = tes_file_writing::subrecord;
}