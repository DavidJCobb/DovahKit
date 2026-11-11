#include "./FormList.h"
#include "dovah/form_data/helpers/subrecord_was_handled_at_stub_creation.h"
#include "dovah/load_order_interfaces/form_load.h"
#include "dovah/load_order_processes/use_info_build/use_info_builder.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "dovah/types/form_id.h"

namespace dovah::form_data_function_tables::build_outbound_use_info {
   void entry<form_type::formlist>::execute(tes::record_view& record, load_order_processes::use_info_build::use_info_builder& uib) {
      using loaded_form_data = form_data::FormList<managed_form_params>;

      if (!uib.is_final_file()) {
         return;
      }
      
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD': // papyrus
               // TODO
               break;
            case 'LNAM': // description
               {
                  form_id id;
                  if (subrecord.read(id))
                     uib.add_outbound_form_use(id);
               }
               break;
         }
      }
   }
}
namespace dovah::form_data_function_tables::serialize_form_data {
   extern void load(form_data::FormList<managed_form_params>& self, tes::record_view& record, load_order_interfaces::form_load& intfc) {
      if (!intfc.is_winning_record) {
         return;
      }
      while (auto& subrecord = record.next_subrecord()) {
         if (form_data_helpers::subrecord_was_handled_at_stub_creation(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               // TODO
               break;
            case 'LNAM':
               {
                  managed_form_use use;
                  if (subrecord.read(use) && use.get()) {
                     self.contents.unmanaged_push_back(std::move(use));
                  }
               }
               break;
            case 'OBND': // no-op
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
}