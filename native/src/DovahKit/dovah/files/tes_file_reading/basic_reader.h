#pragma once
#include <array>
#include <optional>
#include "elements.h"

namespace dovah {
   enum class game;
   class localized_string_store;
}

namespace dovah::tes_file_reading {
   class file_or_file_part_loader;
   class file_loader;

   class basic_reader {
      //
      // A class capable of parsing groups, records, and subrecords. It's not strictly suitable 
      // for loading content; it doesn't have anything built-in for passing record content to 
      // (file_load_order) or a similar class, or otherwise for loading and storing form data 
      // properly; it's only suitable for reading raw file data.
      //
      friend group;
      friend record;
      friend subrecord;
      public:
         enum class object_type {
            none,
            group,
            record,
         };
         //
         static constexpr int max_group_depth = 7;
         //
      protected:
         uint32_t  stream_position = 0;
         std::array<group, max_group_depth> _groups;
         record    _record;
         subrecord _subrecord;
         uint32_t  last_potential_group_parent = 0; // form ID: CELL, WRLD, DIAL
         
         void read(void* buffer, uint32_t size) {
            this->_read_impl(buffer, size);
         }
         void read(char* buffer, uint32_t size) {
            this->_read_impl(buffer, size);
         }
         template<typename T> void read(T& field, uint32_t size) {
            this->_read_impl(&field, size);
         }
         template<typename T> void read(T& field) {
            this->_read_impl(&field, sizeof(T));
         }
         void reset_parse_state() {
            for (uint32_t i = 0; i < this->_groups.size(); i++)
               this->_groups[i].reset();
            this->_record.reset();
            this->_subrecord.reset();
         }
         //
         void _read_impl(void* buffer, uint32_t size);
         //
         void _validate_record_signature();
         
      public:
         basic_reader() : _record(*this), _subrecord(*this) {
            for (uint32_t i = 0; i < this->_groups.size(); i++)
               this->_groups[i].initialize(this);
         };
         //
         const uint8_t*  file_data   = nullptr; // file data to read
         uint32_t        file_size   = 0;
         file_loader*    loader      = nullptr; // optional. needed for loading content that requires a load order
         struct {
            bool allow_suspicious_record_signatures = false;
            bool allow_unknown_record_signatures    = false;
            bool log_file_syntax_errors             = true;  // for on-demand form loading, perf boost from disabling this after the initial file load
            bool uses_string_table                  = false; // only used when no (file_loader) is supplied; otherwise you must set this based on the file header's flags
            std::optional<game> current_game;
         } options;
         //
         void     set_position(uint32_t);
         uint32_t get_position() const noexcept;
         void     rewind(uint32_t by);
         void     skip(uint32_t bytecount);
         //
         bool is_available() const noexcept;
         bool is_eof() const noexcept;
         constexpr bool is_in_bounds(uint32_t bytes) const noexcept {
            return ((uint64_t)this->stream_position + bytes) < this->file_size;
         }
         //
         bool load_record_at(uint32_t pos);
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
         constexpr group& get_current_group() {
            for (signed int i = this->_groups.size() - 1; i >= 0; i--) {
               auto& group = this->_groups[i];
               if (group)
                  return group;
            }
            //
            // We have to return a group& even if we're not in one, but groups have an 
            // operator bool, so you can do
            //
            // if (auto& g = file->getCurrentGroup()) {
            //    //
            //    // ...
            //    //
            // }
            //
            return this->_groups[0];
         }
         constexpr record&    get_current_record()    { return this->_record; }
         constexpr subrecord& get_current_subrecord() { return this->_subrecord; }
         
         bool uses_string_table() const noexcept;
   };
}