#include "legacy_script.h"
#include "../_common_cpp.h"
#include "../../notice_code_list.h"

namespace dovah::loaded_forms::components {
   bool legacy_script::empty() const noexcept {
      if (this->quest)
         return false;
      if (this->header.unk00)
         return false;
      if (this->header.refs || this->header.size || this->header.vars)
         return false;
      if (!this->compiled_data.empty())
         return false;
      if (!this->source_code.empty())
         return false;
      if (!this->refs.empty())
         return false;
      return true;
   }
   void legacy_script::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case 'SCHR':
            if (subrecord.is_in_bounds(0x14)) {
               subrecord.unchecked_read(this->header.unk00);
               subrecord.unchecked_read(this->header.refs);
               subrecord.unchecked_read(this->header.size);
               subrecord.unchecked_read(this->header.vars);
               subrecord.unchecked_read(this->header.type);
            }
            break;
         case 'SCDA':
            this->compiled_data.clear();
            this->compiled_data.resize(subrecord.size());
            for (size_t i = 0; i < subrecord.size(); ++i)
               subrecord.unchecked_read(this->compiled_data[i]);
            if (subrecord.size() != this->header.size) {
               //
               // if we cared about ObScript content, here's where we'd warn
               //
            }
            break;
         case 'SCTX':
            subrecord.read(this->source_code);
            break;
         case 'QNAM':
            subrecord.read(this->quest);
            intfc.log_load_warning(
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::quest, intfc.target_stub, this->quest)
            );
            break;
         case 'SCRO':
            {
               form_reference_t form;
               if (subrecord.read(form)) {
                  this->refs.push_back(form);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_not_object_reference(subrecord.signature(), intfc.target_stub, form)
                  );
               }
            }
            break;
         case 'SCRV':
            //
            // TODO
            //
            break;
         default:
            assert(false && "ContainerData::load should only be called for COCT, CNTO, and COED subrecords!");
      }
   }
   bool legacy_script::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (this->empty())
         return true;
      auto& SCHR = record.open_next_subrecord('SCHR');
      SCHR.write(this->header.unk00);
      SCHR.write(this->header.refs);
      SCHR.write(this->header.size);
      SCHR.write(this->header.vars);
      SCHR.write(this->header.type);
      SCHR.close();
      if (!this->compiled_data.empty()) {
         auto& SCDA = record.open_next_subrecord('SCDA');
         for(const auto byte : this->compiled_data)
            SCDA.write(byte);
         SCDA.close();
      }
      if (!this->source_code.empty()) {
         record.write_string_subrecord('SCTX', this->source_code);
      }
      record.write_formID_subrecord('QNAM', this->quest, true);
      for (const auto& id : this->refs)
         record.write_formID_subrecord('SCRO', id, true);
      return true;
   }
   void legacy_script::clone_from(const legacy_script& other, loaded_forms::Form& my_owner) noexcept {
      this->clear(my_owner);
      //
      this->header = other.header;
      this->compiled_data = other.compiled_data;
      this->source_code   = other.source_code;
      this->quest = other.quest;
      //
      size_t size = other.refs.size();
      this->refs.resize(size);
      for (size_t i = 0; i < size; ++i)
         this->refs[i].set(my_owner, other.refs[i]);
   }
   void legacy_script::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->quest.clear_if(my_owner, target);
      remove_form_from_reference_list(this->refs, target, my_owner);
   }
   void legacy_script::clear(loaded_forms::Form& my_owner) {
      this->quest.set(my_owner, nullptr);
      clear_form_reference_list(this->refs, my_owner);
   }
}