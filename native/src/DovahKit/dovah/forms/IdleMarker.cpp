#include "IdleMarker.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/idle_marker/incorrect_idle_count.h"
#include "../notices/form_save_errors/by_form_type/idle_marker/too_many_idles.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::idle_marker;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::idle_marker;
   }
}

namespace dovah::loaded_forms {
   void IdleMarker::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      uint8_t expected_anim_count = 0;
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
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'IDLF':
               subrecord.read(this->flags);
               break;
            case 'IDLC':
               subrecord.read(expected_anim_count);
               break;
            case 'IDLT':
               subrecord.read(this->timer);
               break;
            case 'IDLA':
               if (subrecord.size() != expected_anim_count * 4) {
                  specific_load_warnings::incorrect_idle_count notice(
                     this->stub,
                     expected_anim_count,
                     subrecord.size() / 4
                  );
                  intfc.log_load_warning(notice);
                  break;
               }
               this->idles.resize(expected_anim_count);
               for (uint8_t i = 0; i < expected_anim_count; ++i) {
                  auto& form = this->idles.emplace_back();
                  if (!subrecord.read(form))
                     break;
                  intfc.warn_if_ref_is_wrong_type(form, form_type::idle, subrecord.signature());
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void IdleMarker::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      uint8_t expected_anim_count = 0;
      std::vector<form_id_t> idles;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'IDLC':
               subrecord.read(expected_anim_count);
               break;
            case 'IDLA':
               if (subrecord.size() != expected_anim_count * 4)
                  break;
               idles.resize(expected_anim_count);
               for (uint8_t i = 0; i < expected_anim_count; ++i) {
                  auto& form = idles.emplace_back();
                  if (!subrecord.read(form))
                     break;
               }
               break;
         }
      }
      for (auto id : idles)
         if (id)
            uib.add_outbound_reference(id);
   }
   void IdleMarker::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (IdleMarker*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);

      copy_form_reference_list(*copy, copy->idles, this->idles);
      copy->timer = this->timer;
      copy->flags = this->flags;
   }
   void IdleMarker::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      {
         auto& subrecord = record.open_next_subrecord('IDLF');
         subrecord.write(this->flags);
         subrecord.close();
      }
      {
         const size_t size = this->idles.size();
         if (size > max_idles_count) {
            auto notice = specific_save_errors::too_many_idles(
               *intfc.target_stub,
               size
            );
            intfc.throw_save_error(notice);
         }
         auto& subrecord = record.open_next_subrecord('IDLC');
         subrecord.write((uint8_t)size);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('IDLT');
         subrecord.write(this->timer);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('IDLA');
         for (auto& idle : this->idles)
            subrecord.write(idle);
         subrecord.close();
      }
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
   }
   void IdleMarker::_clear_impl() noexcept {
      this->bounds.clear();
      this->model.clear(*this);
      this->script_data.clear(*this);

      clear_form_reference_list(this->idles, *this);
      this->timer = 0;
      this->flags = 0;
   }
   void IdleMarker::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      remove_form_from_reference_list(this->idles, other, *this);
   }
}