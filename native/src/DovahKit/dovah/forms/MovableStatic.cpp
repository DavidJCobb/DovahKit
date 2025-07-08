#include "MovableStatic.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void MovableStatic::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
            case 'MODD':
               this->model.load(subrecord, intfc);
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
               subrecord.read(this->flags);
               break;
            case 'SNAM':
               if (auto& form = this->loop_sound; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void MovableStatic::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      form_id_t loop_sound;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'ICON':
            case 'MICO':
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               break;
            case 'SNAM':
               subrecord.read(loop_sound);
               break;
         }
      }
      uib.add_outbound_reference(loop_sound);
      destruction_uib.done();
   }
   void MovableStatic::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MovableStatic*)out;

      copy->bounds = this->bounds;
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
      copy->model.clone_from(this->model, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name  = this->name;
      copy->flags = this->flags;
      copy->loop_sound.set(*copy, this->loop_sound);
   }
   void MovableStatic::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->flags);
         subrecord.close();
      }
      record.write_formID_subrecord('SNAM', this->loop_sound, true);
   }
   void MovableStatic::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->model.clear(*this);
      this->script_data.clear(*this);

      this->loop_sound.set(*this, nullptr);
      this->flags = 0;
      this->name.reset();
   }
   void MovableStatic::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->loop_sound.clear_if(*this, other);
   }
}