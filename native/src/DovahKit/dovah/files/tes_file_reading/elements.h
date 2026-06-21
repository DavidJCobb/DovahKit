#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "helpers/memory.h"
#include "helpers/miscellaneous.h"
#include "helpers/type_traits.h"
#include "helpers/type_traits/is_literal.h"
#include "helpers/type_traits/is_std_array.h"
#include "../../bare_form_id_t.h"
#include "../../form_id_t.h"
#include "../../form_reference_t.h"
#include "../common.h"

namespace dovah {
   class  form_stub;
   struct localized_string;

   namespace tes_file_reading {
      class basic_reader;
      class file_loader;
      class subrecord;
      class record;

      template<typename T> concept IsLiteralIsh = cobb::is_literal<T> || (std::is_bounded_array_v<T> && cobb::is_literal<std::remove_extent_t<T>>);

      #pragma region Classes used to read specific elements of an ESP file (e.g. records, subrecords)
      class group {
         friend basic_reader;
         protected:
            inline void initialize(basic_reader* f) noexcept { this->owner = f; }
         public:
            using type     = dovah::tes_file_group_type;
            using header_t = dovah::tes_file_group_header;
            //
         public:
            basic_reader* owner = nullptr;
            header_t      header;
            uint32_t      pos = 0;
            uint32_t      end = 0;
            
            constexpr operator bool() const noexcept { return this->header.signature != 0; }
            constexpr bool exists() const noexcept { return this->header.signature != 0; }
            
            uint32_t depth() const noexcept;
            void to_string(std::string&) const noexcept;
            
            group* get_parent() const noexcept;
            
            constexpr uint32_t getRawIDOfParentCell() const noexcept {
               switch (this->header.type) {
                  case type::cell_children:
                  case type::cell_persistent_children:
                  case type::cell_temporary_children:
                     return this->header.label;
               }
               return 0;
            }
            constexpr uint32_t getRawIDOfParentTopic() const noexcept {
               if (this->header.type == type::topic_children)
                  return this->header.label;
               return 0;
            }
            
            void reset() {
               this->header.signature = 0;
            }
            void skip();
      };
      class record {
         friend basic_reader;
         friend file_loader;
         friend subrecord;
         public:
            using header_t = dovah::tes_file_record_header;
         protected:
            basic_reader& owner;
            header_t      header;
            uint32_t      head_pos = 0; // position in the file (start of the record)
            uint32_t      body_pos = 0; // position in the file (start of the record body)
            uint32_t      end      = 0;
            //
            cobb::generic_buffer data; // record body (uncompressed)
            uint32_t offset = 0; // offset for reading, within the uncompressed record body
            //
            record(basic_reader& file) : owner(file) {}
            //
         public:
            //
            // Disallow copying to avoid bad memory management on the generic_buffer.
            //
            record& operator=(const record& other) = delete; // no copy
            record(record& other) = delete; // no copy
            //
            subrecord& get_current_subrecord() const noexcept;
            bool is_skyrim_special() const noexcept;
            //
            constexpr operator bool() const noexcept { return this->header.signature != 0; }
            constexpr bool exists() const noexcept { return this->header.signature != 0; }
            //
            inline bool is_in_bounds() const noexcept {
               return this->offset < this->data.size();
            }
            inline bool is_in_bounds(uint32_t room_for) const noexcept {
               return this->offset + room_for <= this->data.size();
            }
            constexpr uint32_t flags() const noexcept { return this->header.flags; }
            constexpr uint32_t formID() const noexcept { return this->header.formID; }
            constexpr uint32_t signature() const noexcept { return this->header.signature; }
            constexpr uint32_t size() const noexcept { return this->header.size; }
            constexpr uint16_t version() const noexcept { return this->header.version; }
            
            constexpr uint32_t header_pos() const noexcept { return this->head_pos; }
            constexpr uint32_t stream_pos() const noexcept { return this->offset + this->body_pos; }
            constexpr uint32_t current_offset() const noexcept { return this->offset; } // how far into the (decompressed) data have we read?
            
            bool read(void* destination, uint32_t size);
            inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
            template<typename T> inline bool read(T& field) {
               return this->read(&field, sizeof(T));
            }
            bool skip(uint32_t bytes);
            //
            void unchecked_read(void* destination, uint32_t size);
            template<typename T> inline void unchecked_read(T& field) {
               this->unchecked_read(&field, sizeof(T));
            }
            
            constexpr bool body_is_compressed() const noexcept { return this->header.body_is_compressed(); }
            
            void return_to_start();
            subrecord& next_subrecord() const;
            uint32_t peek_next_subrecord_type();
            
            void reset();
            void go_to_offset(uint32_t offset) {
               this->offset = offset;
            }
            
            form_stub* lookup_form_by_id(bare_form_id_t) const noexcept;
      };
      class subrecord {
         friend basic_reader;
         public:
            using header_t = dovah::tes_file_subrecord_header;
         protected:
            subrecord(basic_reader& file) : owner(file) {}
            //
            basic_reader& owner;
            header_t header;
            uint32_t body_start = 0; // start of the subrecord body within the containing record
            //
            void _fix_up_form_id(uint32_t& id) const noexcept;
            bool _read_form_id(form_id_t& field) const noexcept;
            bool _read_form_reference(form_reference_t&) const noexcept;
            void _unchecked_read_form_id(form_id_t& field) const noexcept;
            void _unchecked_read_form_reference(form_reference_t&) const noexcept;
            //
         public:
            subrecord& operator=(const subrecord& other) = delete; // no copy
            subrecord(subrecord& other) = delete; // no copy
            
            record& get_containing_record() const;
            inline bool is_skyrim_special() const noexcept { return this->get_containing_record().is_skyrim_special(); }
            inline void reset() {
               this->header.signature = 0;
            }
            
            inline uint32_t offset() const noexcept { return this->get_containing_record().offset - this->body_start; }
            constexpr uint32_t end_pos() const noexcept { return this->body_start + this->header.size; }
            constexpr uint32_t signature() const noexcept { return this->header.signature; }
            constexpr uint32_t size() const noexcept { return this->header.size; }
            
            constexpr operator bool() const { return this->header.signature != 0; }
            constexpr bool exists() const noexcept { return this->header.signature != 0; }
            
            inline bool is_at_end() const {
               return this->get_containing_record().offset == this->end_pos();
            }
            inline bool is_in_bounds() const {
               return this->get_containing_record().offset < this->end_pos();
            }
            inline bool is_in_bounds(uint32_t size) const {
               return this->get_containing_record().offset + size <= this->end_pos();
            }
            
            inline uint32_t containing_record_signature() const { return this->get_containing_record().signature(); }
            
            inline bool skip_bytes(uint32_t count) const { return this->get_containing_record().skip(count); }
            
            #pragma region read
            inline bool read(void* buffer, uint32_t size) const {
               if (!this->is_in_bounds(size))
                  return false;
               return this->get_containing_record().read(buffer, size);
            }
            inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
            //
            template<typename T> requires (IsLiteralIsh<T> || cobb::is_std_array<T>) inline bool read(T& field) const {
               if constexpr (cobb::is_std_array<T>) {
                  if (!this->is_in_bounds(sizeof(T::value_type) * field.size()))
                     return false;
                  for (auto& e : field)
                     this->unchecked_read(e);
                  return true;
               } else {
                  if (!this->is_in_bounds(sizeof(field)))
                     return false;
                  return this->get_containing_record().read(field);
               }
            }
            //
            bool read(std::string& field) const noexcept;
            bool read(localized_string& field) const noexcept;
            inline bool read(form_reference_t& field) const noexcept { return this->_read_form_reference(field); }
            inline bool read(form_id_t& field) const { return this->_read_form_id(field); }
            #pragma endregion
            
            #pragma region unchecked_read
            //
            // The functions below allow you to manually manage bounds-checking: if you need to read multiple 
            // fields in sequence, then it might be a millisecond or two faster to do a single bounds-check 
            // at the start, and then do unchecked reads for the fields, e.g.
            //
            //    uint32_t foo;
            //    uint32_t bar;
            //    if (!subrecord.is_in_bounds(sizeof(foo) + sizeof(bar)))
            //       return false;
            //    subrecord.unchecked_read(foo);
            //    subrecord.unchecked_read(bar);
            //
            // Of course, you'll have to be careful if you go copying and pasting read code. Is that risk 
            // worth a few milliseconds per form, over thousands of forms? Sounds like it to me, but I can 
            // always redesign if it turns out to cause too many problems to be worth it.
            //
            template<typename T> requires (IsLiteralIsh<T> || cobb::is_std_array<T>) inline void unchecked_read(T& field) const {
               if constexpr (cobb::is_std_array<T>) {
                  for (auto& e : field)
                     this->unchecked_read(e);
               } else {
                  this->get_containing_record().unchecked_read(field);
               }
            }
            inline void unchecked_read(form_reference_t& field) const noexcept { this->_unchecked_read_form_reference(field); }
            inline void unchecked_read(form_id_t& field) const { this->_unchecked_read_form_id(field); }
            #pragma endregion
            
            bool read_signature(uint32_t& out) const noexcept;
            
            bool read_wstring(std::string& field); // uint16_t length; char str[length]; // length does not include a null-terminator
            bool read_wstring(std::wstring& field);
            
            template<int length_bytes> inline bool read_length_prefixed_string(std::string& field) const noexcept {
               //
               // Read a string prefixed with a length, with no null terminator.
               //
               using int_t = cobb::bytecount_to_int_t<length_bytes>;
               field.clear();
               int_t length;
               if (this->read(length)) {
                  field.resize(length);
                  return this->read((void*)field.data(), length);
               }
               return false;
            }
            template<int length_bytes> inline bool skip_length_prefixed_string() const noexcept {
               //
               // Skip a string prefixed with a length, with no null terminator.
               //
               using int_t = cobb::bytecount_to_int_t<length_bytes>;
               int_t length;
               if (this->read(length))
                  return this->skip_bytes(length);
               return false;
            }
            
            void back_to_start() {
               this->get_containing_record().go_to_offset(this->body_start);
            }
            void seek(size_t s) {
               this->get_containing_record().go_to_offset(this->body_start + s);
            }
            
            form_stub* lookup_form_by_id(bare_form_id_t) const noexcept;
            //
            bool form_id_can_survive_redundant_fixup(bare_form_id_t) const noexcept;
      };
      #pragma endregion
   }

   using tes_record_reader    = tes_file_reading::record;
   using tes_subrecord_reader = tes_file_reading::subrecord;
}