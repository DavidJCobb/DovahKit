#include "HeadPart.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/head_part/invalid_morph_type.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::head_part;
   }
}

namespace {
   enum class morph_type : uint32_t {
      race,
      tri,
      chargen,
   };
}

namespace dovah::loaded_forms {
   void HeadPart::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      
      auto current_morph_type = morph_type::race;
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
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               break;
            case 'PNAM':
               subrecord.read(this->type);
               break;
            case 'HNAM':
               {
                  form_reference_t form_id = 0;
                  if (subrecord.read(form_id))
                     this->extra_parts.push_back(form_id);
               }
               break;
            case 'NAM0':
               {
                  morph_type v;
                  if (subrecord.read(v)) {
                     switch (v) {
                        case morph_type::race:
                        case morph_type::tri:
                        case morph_type::chargen:
                           current_morph_type = v;
                           break;
                        default:
                           specific_load_warnings::invalid_morph_type notice(
                              this->stub,
                              (uint32_t)v
                           );
                           intfc.log_load_warning(notice);
                           break;
                     }
                  }
               }
               break;
            case 'NAM1':
               {
                  std::string* target = &this->morphs.race;
                  switch (current_morph_type) {
                     case morph_type::race:
                        target = &this->morphs.race;
                        break;
                     case morph_type::tri:
                        target = &this->morphs.tri;
                        break;
                     case morph_type::chargen:
                        target = &this->morphs.chargen;
                        break;
                  }
                  subrecord.read(*target);
               }
               break;
            case 'TNAM':
               subrecord.read(this->texture_set);
               break;
            case 'CNAM':
               subrecord.read(this->color);
               break;
            case 'RNAM':
               subrecord.read(this->valid_races);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void HeadPart::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      form_id_t color;
      form_id_t texture_set;
      form_id_t valid_races;

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
            case 'DATA':
            case 'PNAM':
            case 'NAM0':
            case 'NAM1':
               break;
            case 'HNAM':
               {
                  form_id_t form_id = 0;
                  subrecord.read(form_id);
                  uib.add_outbound_reference(form_id);
               }
               break;
            case 'TNAM':
               subrecord.read(texture_set);
               break;
            case 'CNAM':
               subrecord.read(color);
               break;
            case 'RNAM':
               subrecord.read(valid_races);
               break;
         }
      }
      uib.add_outbound_reference(color);
      uib.add_outbound_reference(texture_set);
      uib.add_outbound_reference(valid_races);
   }
   void HeadPart::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (HeadPart*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->model.clone_from(this->model, *copy);
      //
      copy->name = this->name;
      copy->type = this->type;
      copy->flags = this->flags;
      copy->color.set(*copy, this->color);
      copy->texture_set.set(*copy, this->texture_set);
      copy->valid_races.set(*copy, this->valid_races);
      copy->morphs = this->morphs;
      //
      size_t size = this->extra_parts.size();
      copy->extra_parts.resize(size);
      for (size_t i = 0; i < size; ++i)
         copy->extra_parts[i].set(*copy, this->extra_parts[i]);
   }
   void HeadPart::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->flags);
      DATA.close();
      auto& PNAM = record.open_next_subrecord('PNAM');
      PNAM.write(this->type);
      PNAM.close();
      for (auto& extra : this->extra_parts)
         record.write_formID_subrecord('HNAM', extra.get_form_stub(), true);
      if (auto& path = this->morphs.race; !path.empty()) {
         auto& NAM0 = record.open_next_subrecord('NAM0');
         NAM0.write(morph_type::race);
         NAM0.close();
         record.write_string_subrecord('NAM1', path);
      }
      if (auto& path = this->morphs.tri; !path.empty()) {
         auto& NAM0 = record.open_next_subrecord('NAM0');
         NAM0.write(morph_type::tri);
         NAM0.close();
         record.write_string_subrecord('NAM1', path);
      }
      if (auto& path = this->morphs.chargen; !path.empty()) {
         auto& NAM0 = record.open_next_subrecord('NAM0');
         NAM0.write(morph_type::chargen);
         NAM0.close();
         record.write_string_subrecord('NAM1', path);
      }
      record.write_formID_subrecord('TNAM', this->texture_set, true);
      record.write_formID_subrecord('CNAM', this->color,       true);
      record.write_formID_subrecord('RNAM', this->valid_races, true);
   }
   void HeadPart::_clear_impl() noexcept {
      this->name.reset();
      this->flags  = 0;
      this->type   = head_part_type::misc;

      this->model.clear(*this);
      this->script_data.clear(*this);
      this->color.set(*this, nullptr);
      this->texture_set.set(*this, nullptr);
      this->valid_races.set(*this, nullptr);
      this->morphs = {};
      clear_form_reference_list(this->extra_parts, *this);
   }
   void HeadPart::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->color.clear_if(*this, other);
      this->texture_set.clear_if(*this, other);
      this->valid_races.clear_if(*this, other);

      for (auto& e : this->extra_parts)
         e.clear_if(*this, other);
      std::erase_if(this->extra_parts, [](const auto& e) { return !e; });
   }
}