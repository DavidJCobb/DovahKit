#pragma once
#include "../../helpers/files.h"
#include "elements.h"

namespace dovah {
   class  form_stub;

   namespace tes_file_reading {
      class file_reader;
      
      class basic_reader {
         //
         // This class can be used to read file out of an ESP/ESM/ESL file. It can be subclassed to provide 
         // multi-threaded loading functionality.
         //
         friend file_reader;
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
            virtual file_reader* as_file() const noexcept {
               if (this->owner)
                  return this->owner;
               return (file_reader*)this;
            }
            //
            static constexpr int max_group_depth = 7;
            //
         protected:
            file_reader*          owner = nullptr;
            cobb::mapped_file* file  = nullptr; // NOTE: an instance of basic_reader may not necessarily own the file it has a pointer to
            uint32_t  stream_position = 0;
            std::array<group, max_group_depth> _groups;
            record    _record;
            subrecord _subrecord;
            uint32_t  last_potential_group_parent = 0; // form ID: CELL, WRLD, DIAL
            //
            bool is_skyrim_special = false; // needed for subrecord::read and friends to handle SSE struct form IDs properly
            bool uses_string_table = false;
            //
            void read(void* buffer, uint32_t size) {
               this->stream_position += this->file->read_from(this->stream_position, buffer, size);
            }
            void read(char* buffer, uint32_t size) {
               this->read((void*)buffer, size);
            }
            template<typename T> void read(T& field, uint32_t size) {
               this->stream_position += this->file->read_from(this->stream_position, field, size);
            }
            template<typename T> void read(T& field) {
               this->stream_position += this->file->read_from(this->stream_position, field);
            }
            void resetParseState() {
               for (uint32_t i = 0; i < this->_groups.size(); i++)
                  this->_groups[i].reset();
               this->_record.reset();
               this->_subrecord.reset();
            }
            //
            form_stub* make_stub_for_record(file_reader& file);
            void extract_editor_id_for_stub(form_stub*); // searches (the remainder of) the current record for EDID; if found, writes its value to the form stub
            //
         public:
            basic_reader(file_reader* owner) : owner(owner), _record(*this), _subrecord(*this) {
               for (uint32_t i = 0; i < this->_groups.size(); i++)
                  this->_groups[i].initialize(this);
            };
            //
            void     setPos(uint32_t pos);
            uint32_t getPos();
            void     rewind(uint32_t by);
            void skipBytes(uint32_t count);
            bool isEOF();
            bool is_good();
            //
            object_type next_record_or_group();
            bool        next_subrecord();
            //
            inline group& get_current_group() {
               for (signed int i = this->_groups.size() - 1; i >= 0; i--) {
                  auto& group = this->_groups[i];
                  if (group)
                     return group;
               }
               //
               // We have to return a group& even if we're not in one, but groups have an 
               // operator bool, so you can do
               //
               // if (auto g = file->getCurrentGroup()) {
               //    //
               //    // ...
               //    //
               // }
               //
               return this->_groups[0];
            }
            inline record&    get_current_record()    { return this->_record; }
            inline subrecord& get_current_subrecord() { return this->_subrecord; }
      };
   }
}