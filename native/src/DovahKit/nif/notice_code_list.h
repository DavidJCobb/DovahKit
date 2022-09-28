#pragma once
#include "notice_code_t.h"

namespace nifDK {
   struct notice_code {
      notice_code()       = delete;
      enum type : notice_code_t {
         none = 0x00000000,
         //
         stream_ended_early           = 0x00000001,
         line_string_with_bad_end     = 0x00000002, // line-string ending in "\r" but not "\r\n"
         string_index_out_of_bounds   = 0x00000003,
         multiple_top_level_nodes     = 0x00000004,
         object_has_multiple_parents  = 0x00000005,
         cyclical_node_tree           = 0x00000006,
         bad_block_typename_index     = 0x00000007,
         block_is_abstract_typename   = 0x00000008,
         inconsistent_triangle_counts = 0x00000009,
         unrecognized_endianness      = 0x0000000A, // endianness specified in file header is not recognized
         unsupported_version          = 0x0000000B,
         unsupported_user_version_1   = 0x0000000C,
         unsupported_user_version_2   = 0x0000000D,
         havok_motor_invalid_type     = 0x0000000E,
         havok_constraint_invalid_target_count = 0x0000000F,
      };
   };
}
