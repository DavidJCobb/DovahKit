#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/keyword_list.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"

class TESPluginRecord;

namespace dovah::loaded_forms {
   class Location : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::location;
         Location(const constructor_params& c) : Form(form_type, c) {};

         struct grid_coords {
            int16_t y = 0; // use 32767 for an interior cell
            int16_t x = 0; // use 32767 for an interior cell
         };

         struct enable_point {
            form_reference_t actor;
            form_reference_t enable_parent;
            grid_coords      grid;
            bool removed_by_active_file = false;
         };
         struct exterior_cell_list {
            form_reference_t worldspace;
            std::vector<grid_coords> cells;
            bool removed_by_active_file = false;
         };
         struct persistent_ref {
            form_reference_t actor; // ACHR
            form_reference_t cell_or_world; // interior cell or worldspace
            grid_coords      grid;
            bool removed_by_active_file = false;
         };
         struct unique_ref_entry { // ACUN and LCUN add; RCUN removes
            form_reference_t actor_base;
            form_reference_t actor;
            form_reference_t editor_location; // usually self
            bool removed_by_active_file = false;
         };
         struct special_ref {
            form_reference_t ref_type; // LocRefType
            form_reference_t reference;
            form_reference_t cell_or_world;
            grid_coords      grid;
            bool removed_by_active_file = false;
         };

         components::keyword_list  keywords; // KWDA[KSIZ+]
         components::papyrus_attachment_data script_data; // VMAD
         std::vector<enable_point>       enable_points;   // ACEP/LCEP // RCEP removes
         std::vector<exterior_cell_list> exterior_cells;  // ACEC/LCEC // RCEC removes
         std::vector<form_reference_t>   markers;         // ACID/LCID
         std::vector<persistent_ref>     persistent_refs; // ACPR/LCPR // RCPR removes
         std::vector<special_ref>        special_refs;    // ACSR/LCSR // RCSR removes
         std::vector<unique_ref_entry>   uniques;         // ACUN/LCUN // RCUN removes
         localized_string name; // FULL
         form_reference_t parent_location; // PNAM
         form_reference_t music; // NAM1
         form_reference_t unreported_crime_faction; // FNAM
         form_reference_t marker; // MNAM
         float            radius; // RNAM
         form_reference_t horse_marker; // NAM0
         color_t color; // CNAM

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
   };
}