#include "FootstepSet.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/footstep_set/footstep_count_mismatch.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::footstep_set;
   }
}

namespace dovah::loaded_forms {
   void FootstepSet::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      union {
         std::array<uint32_t, 5> list = {};
         struct {
            uint32_t walk;
            uint32_t run;
            uint32_t sprint;
            uint32_t sneak;
            uint32_t swim;
         };
      } counts;
      //
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'XCNT':
               for (auto& count : counts.list)
                  subrecord.read(count);
               break;
            case 'DATA':
               if (subrecord.size() == 0)
                  break;
               {
                  // We may want to verify more closely whether they clear the arrays or 
                  // just resize them.
                  for (auto& sublist : this->footsteps.sublists)
                     sublist.clear();

                  if (subrecord.size() != (
                     counts.walk +
                     counts.run +
                     counts.sprint +
                     counts.sneak +
                     counts.swim
                  )) {
                     specific_load_warnings::footstep_count_mismatch notice(
                        this->stub,
                        {
                           counts.walk,
                           counts.run,
                           counts.sprint,
                           counts.sneak,
                           counts.swim,
                        },
                        subrecord.size() / 4
                     );
                     intfc.log_load_warning(notice);
                  }

                  auto _load_list = [&intfc, &subrecord](auto& list, uint32_t count) {
                     for (size_t i = 0; i < count; ++i) {
                        auto& item = list.emplace_back();
                        if (subrecord.read(item)) {
                           intfc.warn_if_ref_is_wrong_type(item, form_type::footstep, subrecord.signature());
                        }
                     }
                  };
                  //
                  // The sub-lists are in reverse order.
                  //
                  for (int i = counts.list.size() - 1; i >= 0; --i) {
                     _load_list(this->footsteps.sublists[i], counts.list[i]);
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void FootstepSet::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;

      uint32_t total_count = 0;
      struct {
         uint32_t walk_forward = 0;
         uint32_t run_forward = 0;
         uint32_t walk_forward_alt;
         uint32_t run_forward_alt;
         uint32_t walk_forward_alt_2;
      } counts;
      std::vector<form_id_t> footsteps;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'XCNT':
               total_count = 0;
               for (size_t i = 0; i < 5; ++i) {
                  uint32_t n = 0;
                  subrecord.read(n);
                  total_count += n;
               }
               break;
            case 'DATA':
               if (subrecord.size() == 0)
                  break;
               footsteps.clear();
               footsteps.resize(total_count);
               for (uint32_t i = 0; i < total_count; ++i) {
                  subrecord.read(footsteps[i]);
               }
               break;
         }
      }
      for (auto form_id : footsteps)
         uib.add_outbound_reference(form_id);
   }
   void FootstepSet::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (FootstepSet*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      for (size_t i = 0; i < this->footsteps.sublists.size(); ++i)
         copy_form_reference_list(*copy, copy->footsteps.sublists[i], this->footsteps.sublists[i]);
   }
   void FootstepSet::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('XCNT');
         for (auto& sublist : this->footsteps.sublists)
            subrecord.write((uint32_t)sublist.size());
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         for(auto it = this->footsteps.sublists.rbegin(); it != this->footsteps.sublists.rend(); ++it)
            for (auto& form : *it)
               subrecord.write(form);
         subrecord.close();
      }
   }
   void FootstepSet::_clear_impl() noexcept {
      this->script_data.clear(*this);
      for(auto& sublist : this->footsteps.sublists)
         clear_form_reference_list(sublist, *this);
   }
   void FootstepSet::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& sublist : this->footsteps.sublists)
         remove_form_from_reference_list(sublist, other, *this);
   }
}