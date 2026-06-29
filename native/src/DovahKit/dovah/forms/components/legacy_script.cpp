#include "legacy_script.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   bool legacy_script::use_info_state::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case subrecord_signature_header:
            break;
         case subrecord_signature_compiled_data:
            break;
         case subrecord_signature_source_code:
            break;
         case subrecord_signature_quest:
            subrecord.read(this->parent_quest);
            break;
         case subrecord_signature_variable_decl:
            break;
         case subrecord_signature_variable_name:
            break;
         case subrecord_signature_ref_objects:
            subrecord.read(this->referenced_objects.emplace_back());
            break;
         case subrecord_signature_ref_variables:
            break;
         default:
            return false;
      }
      return true;
   }

   bool legacy_script::empty() const noexcept {
      if (this->parent_quest)
         return false;
      if (this->header.unk00)
         return false;
      if (this->header.ref_obj_count || this->header.compiled_size || this->header.variable_count)
         return false;
      if (this->header.enabled)
         return false;
      if (!this->compiled_data.empty())
         return false;
      if (!this->source_code.empty())
         return false;
      if (!this->referenced_objects.empty())
         return false;
      if (!this->variables.empty())
         return false;
      return true;
   }
   bool legacy_script::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case subrecord_signature_header:
            subrecord.read(this->header.unk00);
            subrecord.read(this->header.ref_obj_count);
            subrecord.read(this->header.compiled_size);
            subrecord.read(this->header.variable_count);
            subrecord.read(this->header.type);
            subrecord.read(this->header.unk11);
            subrecord.read(this->header.enabled);
            break;
         case subrecord_signature_compiled_data:
            this->compiled_data.clear();
            this->compiled_data.resize(subrecord.size());
            subrecord.read(this->compiled_data.data(), subrecord.size());
            if (subrecord.size() != this->header.compiled_size) {
               //
               // if we cared about ObScript content, here's where we'd warn
               //
            }
            break;
         case subrecord_signature_source_code:
            subrecord.read(this->source_code);
            break;
         case subrecord_signature_quest:
            if (subrecord.read(this->parent_quest))
               intfc.warn_if_ref_is_wrong_type(this->parent_quest, form_type::quest, subrecord.signature());
            break;
         case subrecord_signature_variable_decl:
            {
               auto& item = this->variables.emplace_back();
               subrecord.read(item.index);
               subrecord.read(item.unk04);
               subrecord.read(item.unk08);
               subrecord.read(item.unk0C);
               subrecord.read(item.unk10);
               subrecord.read(item.is_long_or_short);
               subrecord.read(item.unk17);
            }
            break;
         case subrecord_signature_variable_name:
            if (!this->variables.empty()) {
               subrecord.read(this->variables.back().name);
            }
            break;
         case subrecord_signature_ref_objects:
            {
               auto& dst = this->referenced_objects.emplace_back().emplace<form_reference_t>();
               subrecord.read(dst);
            }
            break;
         case subrecord_signature_ref_variables:
            {
               auto& dst = this->referenced_objects.emplace_back().emplace<uint32_t>();
               subrecord.read(dst);
            }
            break;
         default:
            return false;
      }
      return true;
   }
   bool legacy_script::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (this->empty())
         return true;
      {
         auto& subrecord = record.open_next_subrecord(subrecord_signature_header);
         subrecord.write(this->header.unk00);
         subrecord.write(this->header.ref_obj_count);
         subrecord.write(this->header.compiled_size);
         subrecord.write(this->header.variable_count);
         subrecord.write(this->header.type);
         subrecord.write(this->header.unk11);
         subrecord.write(this->header.enabled);
         subrecord.close();
      }
      if (auto& data = this->compiled_data; !data.empty()) {
         auto& subrecord = record.open_next_subrecord(subrecord_signature_compiled_data);
         subrecord.write(data.data(), data.size());
         subrecord.close();
      }
      if (auto& data = this->source_code; !data.empty()) {
         record.write_string_subrecord(subrecord_signature_source_code, data);
      }
      record.write_formID_subrecord(subrecord_signature_quest, this->parent_quest, true);
      for (auto& item : this->variables) {
         {
            auto& subrecord = record.open_next_subrecord('SLSD');
            subrecord.write(item.index);
            subrecord.write(item.unk04);
            subrecord.write(item.unk08);
            subrecord.write(item.unk0C);
            subrecord.write(item.unk10);
            subrecord.write(item.is_long_or_short);
            subrecord.write(item.unk17);
            subrecord.close();
         }
         record.write_string_subrecord('SCVR', item.name);
      }
      for (auto& item : this->referenced_objects) {
         if (std::holds_alternative<form_reference_t>(item)) {
            record.write_formID_subrecord('SCRO', std::get<form_reference_t>(item));
         } else {
            auto& subrecord = record.open_next_subrecord('SCRV');
            subrecord.write(std::get<uint32_t>(item));
            subrecord.close();
         }
      }
      return true;
   }
   void legacy_script::clone_from(const legacy_script& other, loaded_forms::Form& my_owner) noexcept {
      this->clear(my_owner);
      
      this->header        = other.header;
      this->compiled_data = other.compiled_data;
      this->source_code   = other.source_code;
      this->parent_quest.set(my_owner, other.parent_quest);

      {
         auto& src_list = other.referenced_objects;
         auto& dst_list = this->referenced_objects;
         const size_t size_prior = dst_list.size();
         const size_t size_after = src_list.size();
         if (size_prior < size_after)
            dst_list.resize(size_after);
         for (size_t i = 0; i < size_after; ++i) {
            auto& src = src_list[i];
            auto& dst = dst_list[i];
            if (std::holds_alternative<form_reference_t>(src)) {
               if (!std::holds_alternative<form_reference_t>(dst)) {
                  dst.emplace<form_reference_t>();
               }
               std::get<form_reference_t>(dst).set(my_owner, std::get<form_reference_t>(src));
            } else {
               if (std::holds_alternative<form_reference_t>(dst)) {
                  std::get<form_reference_t>(dst).set(my_owner, nullptr);
               }
               dst = src;
            }
         }
         if (size_prior > size_after) {
            for (size_t i = size_after; i < size_prior; ++i) {
               auto& dst = dst_list[i];
               if (std::holds_alternative<form_reference_t>(dst)) {
                  std::get<form_reference_t>(dst).set(my_owner, nullptr);
               }
            }
            dst_list.resize(size_after);
         }
      }
      this->variables = other.variables;
   }
   void legacy_script::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->parent_quest.clear_if(my_owner, target);
      {
         auto& list = this->referenced_objects;
         for (auto& item : list)
            if (auto* casted = std::get_if<form_reference_t>(&item))
               casted->clear_if(my_owner, target);
         //
         // For now, don't remove empty entries. We don't know the format of SCPT, e.g. 
         // if the "indices" in the variables list refer to entries in this list.
         //
      }
   }
   void legacy_script::clear(loaded_forms::Form& my_owner) {
      this->parent_quest.set(my_owner, nullptr);
      this->compiled_data.clear();
      this->source_code.clear();
      this->header = {};
      this->parent_quest.set(my_owner, nullptr);
      {
         auto& list = this->referenced_objects;
         for (auto& item : list)
            if (auto* casted = std::get_if<form_reference_t>(&item))
               casted->set(my_owner, nullptr);
         list.clear();
      }
      this->variables.clear();
   }
}