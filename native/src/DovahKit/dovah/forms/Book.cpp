#include "Book.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Book::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case components::object_bounds::subrecord:
               this->bounds.load(subrecord, intfc);
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DESC':
               subrecord.read(this->text);
               break;
            case 'ICON':
               subrecord.read(this->icons.inventory);
               break;
            case 'MICO':
               subrecord.read(this->icons.message);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               subrecord.read(this->type);
               subrecord.skip_bytes(2);
               {
                  if (this->flags & book_flag::teaches_skill) {
                     subrecord.read(this->teaches.emplace<int32_t>());
                  } else {
                     subrecord.read(this->teaches.emplace<form_reference_t>());
                  }
               }
               subrecord.read(this->value);
               subrecord.read(this->weight);
               break;
            case 'YNAM':
               if (auto& form = this->sounds.take; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'ZNAM':
               if (auto& form = this->sounds.drop; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'CNAM':
               subrecord.read(this->description);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Book::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      components::destruction_stage_data::use_info_builder destruction_uib(uib);
      struct {
         form_id_t take;
         form_id_t drop;
      } sounds;
      form_id_t menu_display_object;
      form_id_t teaches_spell;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case components::object_bounds::subrecord:
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'YNAM':
               subrecord.read(sounds.take);
               break;
            case 'ZNAM':
               subrecord.read(sounds.drop);
               break;
            case 'DATA':
               {
                  book_flags_t flags = 0;
                  subrecord.read(flags);
                  subrecord.skip_bytes(4);
                  if (flags & book_flag::teaches_spell)
                     subrecord.read(teaches_spell);
                  else
                     teaches_spell = {};
               }
               break;
            case 'INAM':
               subrecord.read(menu_display_object);
         }
      }
      uib.add_outbound_reference(sounds.take);
      uib.add_outbound_reference(sounds.drop);
      uib.add_outbound_reference(teaches_spell);
      uib.add_outbound_reference(menu_display_object);
      destruction_uib.done();
   }
   void Book::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Book*)out;
      
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
      copy->keywords.clone_from(this->keywords, *copy);
      copy->model.clone_from(this->model, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;
      copy->text = this->text;
      copy->description = this->description;

      copy->icons = this->icons;

      copy->flags = this->flags;
      copy->type  = this->type;
      {
         auto& src = this->teaches;
         auto& dst = copy->teaches;
         if (auto* casted = std::get_if<int32_t>(&src)) {
            dst = *casted;
         } else if (auto* casted = std::get_if<form_reference_t>(&src)) {
            dst.emplace<form_reference_t>().set(*copy, *casted);
         }
      }
      copy->value = this->value;
      copy->weight = this->weight;

      copy->menu_display_object.set(*copy, this->menu_display_object);

      copy->sounds.take.set(*copy, this->sounds.take);
      copy->sounds.drop.set(*copy, this->sounds.drop);
   }
   void Book::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (!this->icons.inventory.empty())
         record.write_string_subrecord('ICON', this->icons.inventory);
      if (!this->icons.message.empty())
         record.write_string_subrecord('MICO', this->icons.message);
      auto& DESC = record.open_next_subrecord('DESC');
      DESC.write(this->text);
      DESC.close();
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('YNAM', this->sounds.take, true);
      record.write_formID_subrecord('ZNAM', this->sounds.drop, true);
      this->keywords.save(record, intfc);
      {
         this->flags &= ~(book_flag::teaches_skill | book_flag::teaches_spell);
         if (std::holds_alternative<int32_t>(this->teaches)) {
            auto skill = std::get<int32_t>(this->teaches);
            if (skill >= 0)
               this->flags |= book_flag::teaches_skill;
         } else if (std::holds_alternative<form_reference_t>(this->teaches)) {
            auto& form = std::get<form_reference_t>(this->teaches);
            if (form)
               this->flags |= book_flag::teaches_spell;
         }

         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->flags);
         subrecord.write(this->type);
         subrecord.skip_bytes(2);
         if (auto* casted = std::get_if<int32_t>(&this->teaches)) {
            subrecord.write(*casted);
         } else if (auto* casted = std::get_if<form_reference_t>(&this->teaches)) {
            subrecord.write(*casted);
         }
         subrecord.write(this->value);
         subrecord.write(this->weight);
         subrecord.close();
      }
      record.write_formID_subrecord('INAM', this->menu_display_object, true);
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->description);
         subrecord.close();
      }
   }
   void Book::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->keywords.clear(*this);
      this->model.clear(*this);
      this->script_data.clear(*this);

      this->icons = {};
      this->sounds.take.set(*this, nullptr);
      this->sounds.drop.set(*this, nullptr);

      this->name.reset();
      this->text.reset();
      this->description.reset();

      this->flags  = 0;
      this->type   = book_type::tome;
      if (auto* casted = std::get_if<form_reference_t>(&this->teaches))
         casted->set(*this, nullptr);
      this->value  = 0;
      this->weight = 0;

      this->menu_display_object.set(*this, nullptr);
   }
   void Book::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->sounds.take.clear_if(*this, other);
      this->sounds.drop.clear_if(*this, other);

      if (auto* casted = std::get_if<form_reference_t>(&this->teaches))
         casted->clear_if(*this, other);
      this->menu_display_object.clear_if(*this, other);
   }
}