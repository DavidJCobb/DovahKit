#pragma once
#include <cstdint>
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
            inline operator bool() const noexcept { return this->header.signature != 0; }
            inline bool exists() const noexcept { return this->header.signature != 0; }
            //
            group* get_parent() const noexcept;

            record& open_next_record();
      };

      class record {
         friend class file_writer;
         public:
            using header_t = dovah::tes_file_record_header;
         protected:
            header_t header;
            cobb::generic_buffer data; // record body (uncompressed)
            //
         public:
            record& operator=(const record& other) = delete; // no copy
            record(record& other) = delete; // no copy
            //
            inline bool is_skyrim_special() const noexcept { return this->header.version >= 44; }
            //
            operator bool() const noexcept { return this->header.signature != 0; }
            inline bool exists() const noexcept { return this->header.signature != 0; }
            //
            subrecord& get_current_subrecord() const noexcept;
            subrecord& open_next_subrecord() const;
            void close();
      };

      class subrecord {
         friend class file_writer;
         public:
            using header_t = dovah::tes_file_subrecord_header;
         protected:
            header_t header;
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
            void close();
      };
   }
}