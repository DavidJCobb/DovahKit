#include "ImpactDataSet.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/impact_data_set/mapping_missing_data.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::impact_data_set;
   }
}

namespace dovah::loaded_forms {
   void ImpactDataSet::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      bool content_loaded = false;
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
            case 'PNAM':
               {
                  auto& m = this->mappings.emplace_back();
                  if (auto& form = m.material_type; subrecord.read(form)) {
                     intfc.warn_if_ref_is_wrong_type(form, form_type::material_type, subrecord.signature());
                  }
                  if (auto& form = m.impact_data; subrecord.read(form)) {
                     intfc.warn_if_ref_is_wrong_type(form, form_type::impact_data, subrecord.signature());
                  }
                  if (!m.material_type || !m.impact_data) {
                     specific_load_warnings::mapping_missing_data notice(
                        this->stub,
                        this->mappings.size() - 1,
                        m.material_type.get_form_stub(),
                        m.impact_data.get_form_stub()
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ImpactDataSet::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'PNAM':
               {
                  form_id_t form;
                  if (subrecord.read(form))
                     uib.add_outbound_reference(form);
                  if (subrecord.read(form))
                     uib.add_outbound_reference(form);
               }
               break;
         }
      }
   }
   void ImpactDataSet::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ImpactDataSet*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      for (auto& m : copy->mappings) {
         m.material_type.set(*copy, nullptr);
         m.impact_data.set(*copy, nullptr);
      }
      copy->mappings.clear();
      //
      copy->mappings.resize(this->mappings.size());
      for (size_t i = 0; i < this->mappings.size(); ++i) {
         auto& src = this->mappings[i];
         auto& dst = copy->mappings[i];
         dst.material_type.set(*copy, src.material_type);
         dst.impact_data.set(*copy, src.impact_data);
      }
   }
   void ImpactDataSet::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (auto& m : this->mappings) {
         auto& subrecord = record.open_next_subrecord('PNAM');
         subrecord.write(m.material_type);
         subrecord.write(m.impact_data);
         subrecord.close();
      }
   }
   void ImpactDataSet::_clear_impl() noexcept {
      this->script_data.clear(*this);

      for (auto& m : this->mappings) {
         m.material_type.set(*this, nullptr);
         m.impact_data.set(*this, nullptr);
      }
      this->mappings.clear();
   }
   void ImpactDataSet::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}