#pragma once
#include "notice_code_t.h"

namespace nifDK {
   struct notice_code {
      notice_code()       = delete;
      enum type : notice_code_t {
         none = 0x00000000,
         //
         stream_ended_early          = 0x00000001,
         line_string_with_bad_end    = 0x00000002, // line-string ending in "\r" but not "\r\n"
         string_index_out_of_bounds  = 0x00000003,
         multiple_top_level_nodes    = 0x00000004,
         object_has_multiple_parents = 0x00000005,
         cyclical_node_tree          = 0x00000006,
         bad_block_typename_index    = 0x00000007,
         block_is_abstract_typename  = 0x00000008,
      };
   };
}
