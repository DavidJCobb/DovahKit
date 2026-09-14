#pragma once
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

      protected:
         void _crawl_special_refs(form_stub& cell_or_world);
         static form_stub* _get_containing_world(form_stub& cell);
         static form_stub* _get_explicit_location(form_stub&);
         static form_stub* _get_loc_ref_type(form_stub&);
         static bool _is_unique_actor(form_stub& base_form);

      protected:
         form_stub* location = nullptr;
         struct {
            form_stub* NoZoneZone = nullptr;
            form_stub* PersistLoc = nullptr; // DOBJ[PLOC]
         } _cache;
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
         void gather(form_stub& location);
         void apply(bool as_base_record);

      protected:
         void _apply_persist_loc_refs(loaded_forms::Location&, bool as_base_record); // *CPR, and by extension *CEP and *CID
         void _apply_exterior_cells(loaded_forms::Location&, bool as_base_record); // *CEC
         void _apply_special_refs(loaded_forms::Location&, bool as_base_record); // *CSR
         void _apply_unique_actors(loaded_forms::Location&, bool as_base_record); // *CUN
   };
}