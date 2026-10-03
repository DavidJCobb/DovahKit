#pragma once
#include <array>
#include "../group_header.h"
#include "./record_view.h"
#include "./subrecord_view.h"
namespace dovah::tes {
   class file;
}

namespace dovah::tes {
   class file_view {
      public:
         file_view();

         enum class chunk_type {
            none,
            group,
            record,
         };

         struct opened_group : public group_header {
            size_t position = 0;
            size_t end      = 0;
         };

      protected:
         bool _is_eof() const;
         void _read_bytes(void* dst, size_t dst_size);

         void _open_group_here(); // call after reading the GRUP signature
         bool _open_record_here(lu::four_cc signature); // call after reading the non-GRUP signature

      public:
         const file* target_file = nullptr;
         std::array<opened_group, 5> groups;
         struct {
            record_view    record;
            subrecord_view subrecord;
         } subviews;
         size_t position = 0;

         opened_group* get_current_group();
         chunk_type next_record_or_group();
   };
}
