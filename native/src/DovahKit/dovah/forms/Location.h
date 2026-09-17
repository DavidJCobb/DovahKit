#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/keyword_list.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"
#include "../use_info/entry_flags/location.h"

class TESPluginRecord;

namespace dovah::loaded_forms {
   class Location : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::location;
         Location(const constructor_params& c) : Form(form_type, c) {};

         template<typename T>
         struct content_list {
            std::vector<T> base; // LC**
            std::vector<T> full; // LC** + AC** - RC**
         };

         struct grid_coords {
            constexpr bool operator==(const grid_coords&) const noexcept = default;
            int16_t y = 0; // use 32767 for an interior cell
            int16_t x = 0; // use 32767 for an interior cell
         };

         #pragma region Content definitions
            struct enable_parent { // *CEP
               constexpr bool operator==(const typename Location::enable_parent&) const noexcept = default;

               struct flag {
                  enum type : uint8_t {
                     opposite = 1 << 0,
                     pop_in   = 1 << 1,
                  };
               };

               form_reference_t ref;
               form_reference_t enable_parent;
               uint8_t          flags = 0;
            };
            struct exterior_cell_list { // *CEC
               form_reference_t worldspace;
               std::vector<grid_coords> cells;
            };
            struct persist_loc_ref { // *CPR
               constexpr bool operator==(const persist_loc_ref&) const noexcept = default;
               form_reference_t ref;           // *CPR+0x00 -> ACHR
               form_reference_t cell_or_world; // (LCPR|ACPR)+0x04 -> CELL|WRLD (interior cell or worldspace)
               grid_coords      grid;          // (LCPR|ACPR)+0x08
            };
            struct unique_actor { // *CUN
               constexpr bool operator==(const unique_actor&) const noexcept = default;
               form_reference_t actor_base;
               form_reference_t actor;
               form_reference_t editor_location; // -> LCTN
            };
            struct special_ref { // *CSR
               constexpr bool operator==(const special_ref&) const noexcept = default;
               form_reference_t ref_type; // LocRefType
               form_reference_t reference;
               form_reference_t cell_or_world;
               grid_coords      grid;
            };
         #pragma endregion

      public:
         components::keyword_list  keywords; // KWDA[KSIZ+]
         components::papyrus_attachment_data script_data; // VMAD
         struct {
            content_list<enable_parent>      enable_parents;     // ACEP/LCEP // RCEP removes
            content_list<exterior_cell_list> exterior_cells;     // ACEC/LCEC // RCEC removes
            content_list<form_reference_t>   initially_disabled; // ACID/LCID
            content_list<persist_loc_ref>    persistent_refs;    // ACPR/LCPR // RCPR removes
            content_list<special_ref>        special_refs;       // ACSR/LCSR // RCSR removes
            content_list<unique_actor>       unique_actors;      // ACUN/LCUN // RCUN removes
         } contents;
         localized_string name; // FULL
         unique_form_reference_t<use_info::entry_flags::location::parent_location> parent_location; // PNAM
         form_reference_t music; // NAM1
         form_reference_t unreported_crime_faction; // FNAM
         form_reference_t marker; // MNAM
         float            radius = 0; // RNAM
         form_reference_t horse_marker; // NAM0
         color_t color; // CNAM

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}