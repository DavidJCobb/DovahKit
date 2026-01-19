#include "./get_region_worldspace.h"
#include <limits>
#include "../files/file_load_order.h"
#include "../files/tes_file_reading/basic_reader.h"
#include "../form_stub.h"
#include "../form_stubs/helpers/get_unique_outbound_use.h"
#include "../use_info/entry_flags/region.h"
#include "./subrecord_data_position.h"

namespace dovah::utils {
   namespace {
      class skimmer_type : public tes_file_reading::basic_reader {
         public:
            
      };
   }

   extern form_stub* get_region_worldspace(form_stub& stub) {
      if (stub.form_type != dovah::form_type::region)
         return nullptr;
      auto* world = form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::region::worldspace>(stub);
      if (world)
         return world;

      struct {
         subrecord_data_position position;
         size_t     file_index = 0;
         form_stub* worldspace = nullptr;
      } earliest;

      auto& lo = stub.get_owning_load_order();
      for (auto& pair : stub.inbound) {
         auto* cell = pair.second.other;
         if (!cell || cell->form_type != dovah::form_type::cell)
            continue;
         auto* world = cell->get_parent_form();
         if (!world || world->form_type != dovah::form_type::worldspace)
            continue;
         //
         // We can't just load the CELL, because this happens when CELL/XCLR is being 
         // loaded, not in TESForm::InitItemImpl. Losing records, and the order in 
         // which they appear, will influence the results.
         //
         size_t file_count = cell->source_file_count();
         for (size_t i = 0; i < file_count; ++i) {
            const auto* file_info = cell->get_source_file_info(i);

            auto file_index = lo.index_of_file(*file_info->pointer);
            if (earliest.position.source_file) {
               if (file_index > earliest.file_index)
                  continue;
               if (file_info->pointer == earliest.position.source_file) {
                  if (file_info->offset > earliest.position.offsets.of_record)
                     continue;
               }
            }
            
            subrecord_data_position this_pos;

            skimmer_type skimmer;
            cell->do_custom_parse(&skimmer, [&stub, &this_pos](dovah::form_stub& cell_stub, tes_file_reading::record& record, load_order_interfaces::form_load& intfc) {
               while (auto& subrecord = record.next_subrecord()) {
                  if (subrecord.signature() != 'XCLR')
                     continue;
                  size_t size = subrecord.size() / 4;
                  for (size_t i = 0; i < size; ++i) {
                     form_reference_t form;
                     subrecord.unchecked_read(form);
                     if (form == &stub) {
                        this_pos = subrecord_data_position::from_loader(subrecord, intfc);
                        break;
                     }
                  }
               }
            });

            if (this_pos.source_file) {
               if (this_pos.source_file == earliest.position.source_file) {
                  if (earliest.position.offsets.of_record < this_pos.offsets.of_record)
                     continue;
                  if (earliest.position.offsets.of_subrecord < this_pos.offsets.of_subrecord)
                     continue;
               }
               earliest.position   = this_pos;
               earliest.file_index = file_index;
               earliest.worldspace = world;
            }
         }
      }
      return earliest.worldspace;
   }
}