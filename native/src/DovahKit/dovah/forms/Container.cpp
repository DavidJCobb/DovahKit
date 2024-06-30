#include "Container.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Container::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
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
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord, intfc);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->container_flags);
               subrecord.read(this->weight);
               break;
            case 'SNAM': // open sound
               if (subrecord.read(this->open_sound)) {
                  intfc.warn_if_ref_is_wrong_type(this->open_sound, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            case 'QNAM': // close sound
               if (subrecord.read(this->close_sound)) {
                  intfc.warn_if_ref_is_wrong_type(this->close_sound, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Container::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      form_id_t sound_open;
      form_id_t sound_close;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM': // open sound
               subrecord.read(sound_open);
               break;
            case 'QNAM': // close sound
               subrecord.read(sound_close);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generate_use_info(subrecord, uib);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DATA': // flags and weight
               break;
         }
      }
      uib.add_outbound_reference(sound_open);
      uib.add_outbound_reference(sound_close);
      destruction_uib.done();
   }
   void Container::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Container*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      copy->inventory.clone_from(this->inventory, *copy);
      {
         auto& src_opt = this->destruction_data;
         auto& dst_opt = copy->destruction_data;
         if (dst_opt.has_value()) {
            dst_opt.value().clear(*copy);
            dst_opt = {};
         }
         if (src_opt.has_value()) {
            dst_opt.emplace();
            dst_opt.value().clone_from(src_opt.value(), *copy);
         }
      }
      copy->container_flags = this->container_flags;
      copy->name   = this->name;
      copy->weight = this->weight;
      copy->open_sound.set(*copy, this->open_sound);
      copy->close_sound.set(*copy, this->close_sound);
   }
   void Container::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      this->inventory.save(record, intfc);
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->container_flags);
      DATA.write(this->weight);
      DATA.close();
      record.write_formID_subrecord('SNAM', this->open_sound, true);
      record.write_formID_subrecord('QNAM', this->close_sound, true);
   }
   void Container::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->inventory.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      //
      this->open_sound.clear_if(*this, other);
      this->close_sound.clear_if(*this, other);
   }
   void Container::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->model.clear(*this);
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->name.reset();
      this->inventory.clear(*this);
      this->container_flags = 0;
      this->weight = 0.0F;
      this->open_sound.set(*this, nullptr);
      this->close_sound.set(*this, nullptr);
   }
}