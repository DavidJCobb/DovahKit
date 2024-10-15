#include "CollisionLayer.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void CollisionLayer::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'BNAM':
               subrecord.read(this->unique_id);
               break;
            case 'FNAM':
               this->debug_color.load(subrecord);
               break;
            case 'GNAM':
               subrecord.read(this->layer_flags);
               break;
            case 'MNAM':
               subrecord.read(this->name);
               break;
            case 'INTV':
               {
                  uint32_t count = 0;
                  if (subrecord.read(count))
                     this->collides_with.reserve(count);
               }
               break;
            case 'CNAM':
               if (subrecord.size() >= sizeof(uint32_t)) {
                  size_t count = subrecord.size() / sizeof(uint32_t);
                  this->collides_with.reserve(this->collides_with.size() + count);
                  for (size_t i = 0; i < count; ++i) {
                     subrecord.read(this->collides_with.emplace_back());
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void CollisionLayer::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'CNAM':
               if (subrecord.size() >= sizeof(uint32_t)) {
                  size_t count = subrecord.size() / sizeof(uint32_t);
                  for (size_t i = 0; i < count; ++i) {
                     form_id_t form_id;
                     if (subrecord.read(form_id) && form_id)
                        uib.add_outbound_reference(form_id);
                  }
               }
               break;
         }
      }
   }
   void CollisionLayer::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (CollisionLayer*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->name        = this->name;
      copy->description = this->description;
      copy->layer_flags = this->layer_flags;
      copy->unique_id   = this->unique_id;
      copy->debug_color = this->debug_color;
      copy_form_reference_list(*copy, copy->collides_with, this->collides_with);
   }
   void CollisionLayer::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& DESC = record.open_next_subrecord('FULL');
         DESC.write(this->description);
         DESC.close();
      }
      {
         auto& BNAM = record.open_next_subrecord('BNAM');
         BNAM.write(this->unique_id);
         BNAM.close();
      }
      {
         auto& FNAM = record.open_next_subrecord('FNAM');
         this->debug_color.save(FNAM);
         FNAM.close();
      }
      {
         auto& GNAM = record.open_next_subrecord('GNAM');
         GNAM.write(this->layer_flags);
         GNAM.close();
      }
      record.write_string_subrecord('MNAM', this->name);
      {
         auto& list = this->collides_with;
         {
            auto& INTV = record.open_next_subrecord('INTV');
            INTV.write((uint32_t)list.size());
            INTV.close();
         }
         {
            auto& CNAM = record.open_next_subrecord('CNAM');
            for (auto& form : list)
               CNAM.write(form);
            CNAM.close();
         }
      }
   }
   void CollisionLayer::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->description.reset();
      this->name.clear();
      this->unique_id   = 0;
      this->debug_color = {};
      this->layer_flags = 0;
      clear_form_reference_list(this->collides_with, *this);
   }
   void CollisionLayer::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      remove_form_from_reference_list(this->collides_with, other, *this);
   }
}