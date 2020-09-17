#pragma once
#include <cstdint>

namespace dovah {
   struct tes_file_flag {
      tes_file_flag() = delete;
      enum type {
         master                 = 0x0001,
         altered                = 0x0002, // listed in xEdit defs. what does it mean?
         checked                = 0x0004, // listed in xEdit defs. what does it mean?
         active                 = 0x0008, // listed in xEdit defs. what does it mean?
         optimized              = 0x0010, // listed in xEdit defs. what does it mean?
         temp_id_owner          = 0x0020, // listed in xEdit defs. what does it mean?
         localized_string_table = 0x0080,
         precalc_data_only      = 0x0100, // listed in xEdit defs. what does it mean?
         light                  = 0x0200, // SSE-only
      };
   };

   #pragma region record groups
   /*//
   /// GROUP HIERARCHIES:
   ///
   /// [GRUP] Forms Of Type: WRLD
   ///    [WRLD] Worldspace record
   ///    [GRUP] World Children
   ///       [CELL] Persistent cell record
   ///       [GRUP] Cell Children
   ///          [GRUP] Cell Persistent Children
   ///             [REFR] Placed objects
   ///          [GRUP] Cell Temporary Children
   ///             [REFR] Placed objects
   ///       [GRUP] Exterior Cell Block
   ///          [GRUP] Exterior Cell Sub-Block
   ///             [CELL] Cell
   ///             ...
   ///          [GRUP] Exterior Cell Sub-Block
   ///             ...
   ///       [GRUP] Exterior Cell Block
   ///          ...
   ///    ...
   /// [GRUP] Forms Of Type: CELL
   ///    [GRUP] Interior Cell Block
   ///       [GRUP] Interior Cell Sub-Block
   ///          [CELL] Cell
   ///          ...
   ///       [GRUP] Interior Cell Sub-Block
   ///          ...
   ///    [GRUP] Interior Cell Block
   ///       ...
   /// [GRUP] Forms Of Type: DIAL
   ///    [DIAL] Topic
   ///    [GRUP] Topic Children
   ///       [INFO] Topic Info
   ///    ...
   /// [GRUP] Forms Of Type: <any other>
   ///    [....] Record
   ///    ...
   ///
   //*/
   enum class tes_file_group_type : int32_t {
      forms_of_type            = 0,
      world_children           = 1,
      interior_cell_block      = 2,
      interior_cell_sub_block  = 3,
      exterior_cell_block      = 4,
      exterior_cell_sub_block  = 5,
      cell_children            = 6,
      topic_children           = 7, // DIAL -> INFO
      cell_persistent_children = 8,
      cell_temporary_children  = 9,
   };
   struct tes_file_group_header {
      using type_t = tes_file_group_type;
      //
      uint32_t signature = 0; // should always be 'GRUP'
      uint32_t size;
      uint32_t label;
      type_t   type;
      union {
         struct {
            uint8_t vc_day;
            uint8_t vc_month;
            uint8_t vc_last_editor;
            uint8_t vc_current_editor;
         };
         uint32_t version_control;
      };
      uint32_t unknown;
   };
   #pragma endregion

   #pragma region records
   struct tes_file_record_header {
      struct flag {
         flag() = delete;
         enum {
            compressed = 0x00040000,
         };
      };
      //
      uint32_t signature = 0;
      uint32_t size;
      uint32_t flags;
      uint32_t formID = 0;
      union {
         struct {
            uint8_t vc_day;
            uint8_t vc_month;
            uint8_t vc_last_editor;
            uint8_t vc_current_editor;
         };
         uint32_t version_control;
      };
      uint16_t version;
      uint16_t unknown;
      //
      inline bool body_is_compressed() const noexcept { return (this->flags & flag::compressed) != 0; }
   };
   #pragma endregion

   #pragma region subrecords
   struct tes_file_subrecord_header {
      uint32_t signature = 0;
      uint32_t size      = 0;
   };
   #pragma endregion
}
