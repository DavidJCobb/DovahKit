#pragma once
#include <cstdint>

namespace dovah {
   struct tes_file_flag {
      tes_file_flag() = delete;
      enum type {
         master                 = 0x0001,
         altered                = 0x0002, // apparently run-time state for forms; indicates a form that has been edited during the current session.
         checked                = 0x0004, // apparently run-time state for files; indicates that a file's header and dependencies have been checked for correctness?
         active                 = 0x0008, // apparently run-time state for forms and files; indicates the active file and any forms defined/overridden therein
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
      none = -1,
      //
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
      uint32_t size  = 0;
      uint32_t label = 0;
      type_t   type  = type_t::none;
      union {
         struct {
            uint8_t vc_day;
            uint8_t vc_month;
            uint8_t vc_last_editor;
            uint8_t vc_current_editor;
         };
         uint32_t version_control = 0;
      };
      uint32_t unknown = 0; // TODO: this can be 1 for some interior CELL groups; why?
      //
      static constexpr uint32_t uninitialized_unknown = 0xCCCCCCCC; // seen in Bethesda content; MSVC can use this for uninitialized memory, especially in Debug
      //
      static constexpr int offset_of_size = 4; // don't use offsetof(); compilers can insert padding bytes anywhere in a struct at their discretion, and we want to be future-proof
      static constexpr int struct_size    = sizeof(signature) + sizeof(size) + sizeof(label) + sizeof(type) + sizeof(version_control) + sizeof(unknown);
   };
   #pragma endregion

   #pragma region records
   struct tes_file_record_header {
      struct flag {
         flag() = delete;
         enum {
            deleted    = 0x00000020,
            persistent = 0x00000400, // only used for some forms, including REFR and CELL
            ignored    = 0x00001000, // used at run-time in the Creation Kit; prevents a form from being saved; shouldn't appear in files
            partial    = 0x00004000,
            compressed = 0x00040000,
         };
      };
      static constexpr uint32_t non_data_flags = flag::compressed; // when serializing a form, the serializer should decide whether these flags remain set
      //
      uint32_t signature = 0;
      uint32_t size      = 0;
      uint32_t flags     = 0;
      uint32_t formID    = 0;
      union {
         struct {
            uint8_t vc_day;
            uint8_t vc_month;
            uint8_t vc_last_editor;
            uint8_t vc_current_editor;
         };
         uint32_t version_control = 0;
      };
      uint16_t version = 0;
      uint16_t version_control_2 = 0;
      //
      constexpr bool body_is_compressed() const noexcept { return (this->flags & flag::compressed) != 0; }
      //
      static constexpr int struct_size = sizeof(signature) + sizeof(size) + sizeof(flags) + sizeof(formID) + sizeof(version_control) + sizeof(version) + sizeof(version_control_2);
   };
   #pragma endregion

   #pragma region subrecords
   struct tes_file_subrecord_header {
      uint32_t signature = 0;
      uint32_t size      = 0; // in an actual file, subrecords use uint16_t, but we use uint32_t in-memory to handle extended subrecords (i.e. those that use the 'XXXX' system)
      //
      static constexpr int struct_size = sizeof(signature) + 2;
   };
   #pragma endregion
}
