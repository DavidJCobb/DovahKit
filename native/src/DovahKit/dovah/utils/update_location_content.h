#pragma once
#include <cstdint>
#include <vector>

namespace dovah {
   namespace loaded_forms {
      class Location;
      class ObjectReference;
   }
   class file_load_order;
   class form_stub;
}

namespace dovah::utils {
   struct update_location_content {
      public:
         struct exterior_cell_list {
            form_stub* worldspace = nullptr;
            std::vector<form_stub*> cells;
         };

         class location_update_during_save_passkey {
            friend loaded_forms::Location;
            private:
               constexpr location_update_during_save_passkey() {}
         };

      protected:
         void _crawl_special_refs(const form_stub& cell_or_world);
         static form_stub* _get_containing_world(const form_stub& cell);
         static form_stub* _get_explicit_location(const form_stub&);
         static form_stub* _get_loc_ref_type(const form_stub&);

         // Checks that require being able to load forms mid-save:
         std::pair<dovah::form_stub*, uint8_t> _get_enable_parent_info(form_stub& refr); // parent + flags
         bool _is_unique_actor(form_stub& base_form);

      protected:
         form_stub* location = nullptr;
         struct {
            const form_stub* NoZoneZone = nullptr;
            const form_stub* PersistLoc = nullptr; // DOBJ[PLOC]
         } _cache;
         bool _is_mid_save = false;
      public:
         struct {
            std::vector<form_stub*> persist_loc_refs;
            std::vector<form_stub*> special_refs;
            std::vector<form_stub*> unique_actors;
            std::vector<exterior_cell_list> exterior_cell_lists;
         } content;

      protected:
         void _recache_notable_forms(file_load_order&);
      public:
         void _set_is_mid_save_location_fixup(location_update_during_save_passkey); // HACK HACK HACK; see `_is_unique_actor` and enable-parent checks in `_apply_persist_loc_refs`
         void gather(form_stub& location);
         void apply(bool as_base_record);

      protected:
         void _apply_persist_loc_refs(loaded_forms::Location&, bool as_base_record); // *CPR, and by extension *CEP and *CID
         void _apply_exterior_cells(loaded_forms::Location&, bool as_base_record); // *CEC
         void _apply_special_refs(loaded_forms::Location&, bool as_base_record); // *CSR
         void _apply_unique_actors(loaded_forms::Location&, bool as_base_record); // *CUN
   };
}