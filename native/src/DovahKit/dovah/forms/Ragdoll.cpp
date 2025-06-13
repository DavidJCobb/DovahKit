#include "Ragdoll.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Ragdoll::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'MODL':
            case 'MODT':
               this->model.load(subrecord, intfc);
               break;
            case 'NVER':
               subrecord.read(this->version);
               break;
            case 'DATA':
               subrecord.read(this->data.unk00);
               subrecord.read(this->data.unk02);
               subrecord.read(this->data.unk04);
               subrecord.read(this->data.unk06);
               subrecord.read(this->data.unk08);
               subrecord.read(this->data.unk09);
               subrecord.read(this->data.unk0A);
               subrecord.read(this->data.unk0B);
               subrecord.read(this->data.unk0C);
               subrecord.read(this->data.unk0D);
               this->rafb.resize(this->data.unk00);
               break;
            case 'XNAM':
               if (auto& form = this->preview_actor; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::actor_base, subrecord.signature());
               break;
            case 'TNAM':
               if (auto& form = this->body_part_data; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::body_part_data, subrecord.signature());
               break;
            case 'RAFD':
               subrecord.read(this->rafd.unk00);
               subrecord.read(this->rafd.unk04);
               subrecord.read(this->rafd.unk08);
               subrecord.read(this->rafd.unk0C);
               subrecord.read(this->rafd.unk10);
               subrecord.read(this->rafd.unk14);
               subrecord.read(this->rafd.unk18);
               subrecord.read(this->rafd.unk1C);
               subrecord.read(this->rafd.unk20);
               subrecord.read(this->rafd.unk24);
               subrecord.read(this->rafd.unk28);
               subrecord.read(this->rafd.unk2C);
               subrecord.read(this->rafd.unk30);
               subrecord.read(this->rafd.unk34);
               subrecord.read(this->rafd.unk38);
               break;
            case 'RAFB': // present only if not empty
               //
               // We *should* probably warn if the size of this subrecord doesn't match the 
               // number of elements declared by DATA+0x00... but how would we even word a 
               // warning, were we to display it? "Hey, user, we have no idea what any of 
               // this is or does, but these fields, which you're equally clueless about, 
               // don't match."
               //
               for (auto& v : this->rafb)
                  subrecord.read(v);
               break;
            case 'RAPS':
               subrecord.read(this->raps.unk00);
               subrecord.read(this->raps.unk02);
               subrecord.read(this->raps.unk04);
               subrecord.read(this->raps.unk06);
               subrecord.read(this->raps.unk08);
               subrecord.read(this->raps.unk0C);
               subrecord.read(this->raps.unk10);
               subrecord.read(this->raps.unk14);
               break;
            case 'ANAM':
               subrecord.read(this->anam);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Ragdoll::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      form_id_t body_part_data;
      form_id_t preview_actor;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'TNAM':
               subrecord.read(body_part_data);
               break;
            case 'XNAM':
               subrecord.read(preview_actor);
               break;
         }
      }
      uib.add_outbound_reference(body_part_data);
      uib.add_outbound_reference(preview_actor);
   }
   void Ragdoll::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Ragdoll*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->anam = this->anam;
      copy->data = this->data;
      copy->rafb = this->rafb;
      copy->rafd = this->rafd;
      copy->raps = this->raps;

      copy->body_part_data.set(*copy, this->body_part_data);
      copy->preview_actor.set(*copy, this->preview_actor);
   }
   void Ragdoll::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT');
      {
         auto& subrecord = record.open_next_subrecord('NVER');
         subrecord.write(this->version);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->data.unk00);
         subrecord.write(this->data.unk02);
         subrecord.write(this->data.unk04);
         subrecord.write(this->data.unk06);
         subrecord.write(this->data.unk08);
         subrecord.write(this->data.unk09);
         subrecord.write(this->data.unk0A);
         subrecord.write(this->data.unk0B);
         subrecord.write(this->data.unk0C);
         subrecord.write(this->data.unk0D);
         subrecord.close();
      }
      record.write_formID_subrecord('XNAM', this->preview_actor, true);
      record.write_formID_subrecord('TNAM', this->body_part_data, true);
      {
         auto& subrecord = record.open_next_subrecord('RAFD');
         subrecord.write(this->rafd.unk00);
         subrecord.write(this->rafd.unk04);
         subrecord.write(this->rafd.unk08);
         subrecord.write(this->rafd.unk0C);
         subrecord.write(this->rafd.unk10);
         subrecord.write(this->rafd.unk14);
         subrecord.write(this->rafd.unk18);
         subrecord.write(this->rafd.unk1C);
         subrecord.write(this->rafd.unk20);
         subrecord.write(this->rafd.unk24);
         subrecord.write(this->rafd.unk28);
         subrecord.write(this->rafd.unk2C);
         subrecord.write(this->rafd.unk30);
         subrecord.write(this->rafd.unk34);
         subrecord.write(this->rafd.unk38);
         subrecord.close();
      }
      if (!this->rafb.empty()) {
         auto& subrecord = record.open_next_subrecord('RAFB');
         for (auto& v : this->rafb)
            subrecord.write(v);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('RAPS');
         subrecord.write(this->raps.unk00);
         subrecord.write(this->raps.unk02);
         subrecord.write(this->raps.unk04);
         subrecord.write(this->raps.unk06);
         subrecord.write(this->raps.unk08);
         subrecord.write(this->raps.unk0C);
         subrecord.write(this->raps.unk10);
         subrecord.write(this->raps.unk14);
         subrecord.close();
      }
      record.write_string_subrecord('ANAM', this->anam);
   }
   void Ragdoll::_clear_impl() noexcept {
      this->model.clear();
      this->script_data.clear(*this);

      this->anam.clear();
      this->data = {};
      this->rafb.clear();
      this->rafd = {};
      this->raps = {};

      this->body_part_data.set(*this, nullptr);
      this->preview_actor.set(*this, nullptr);
   }
   void Ragdoll::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->body_part_data.clear_if(*this, other);
      this->preview_actor.clear_if(*this, other);
   }
}