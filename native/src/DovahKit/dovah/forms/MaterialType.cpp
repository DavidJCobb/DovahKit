#include "MaterialType.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/material_type/name_too_long.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::material_type;
   }
}

namespace dovah::loaded_forms {
   void MaterialType::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'PNAM':
               if (auto& form = this->parent; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::material_type, subrecord.signature());
               break;
            case 'MNAM':
               if (subrecord.read(this->name)) {
                  auto size = this->name.size();
                  if (size > max_name_length) {
                     specific_load_warnings::name_too_long notice(
                        this->stub,
                        size
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'CNAM':
               subrecord.read(this->color.r);
               subrecord.read(this->color.g);
               subrecord.read(this->color.b);
               break;
            case 'BNAM':
               subrecord.read(this->buoyancy);
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'HNAM':
               if (auto& form = this->impact_data_set; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::impact_data_set, subrecord.signature());
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void MaterialType::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      form_id_t parent;
      form_id_t impact_data_set;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'PNAM':
               subrecord.read(parent);
               break;
            case 'HNAM':
               subrecord.read(impact_data_set);
               break;
         }
      }
      uib.add_outbound_reference(parent);
      uib.add_outbound_reference(impact_data_set);
   }
   void MaterialType::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MaterialType*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->flags = this->flags;
      copy->name = this->name;
      copy->color = this->color;
      copy->buoyancy = this->buoyancy;
      copy->parent.set(*copy, this->parent);
      copy->impact_data_set.set(*copy, this->impact_data_set);
   }
   void MaterialType::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      record.write_formID_subrecord('PNAM', this->parent, true);
      record.write_string_subrecord('MNAM', this->name);
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->color.r);
         subrecord.write(this->color.g);
         subrecord.write(this->color.b);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('BNAM');
         subrecord.write(this->buoyancy);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('FNAM');
         subrecord.write(this->flags);
         subrecord.close();
      }
      record.write_formID_subrecord('HNAM', this->impact_data_set, true);
   }
   void MaterialType::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->flags = 0;
      this->name.clear();
      this->color = {};
      this->buoyancy = 0;
      this->parent.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
   }
   void MaterialType::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->parent.clear_if(*this, other);
      this->impact_data_set.clear_if(*this, other);
   }
}