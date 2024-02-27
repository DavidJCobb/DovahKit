#include "./attachment_data.h"
#include "../../_common_cpp.h"
#include "../../../logging.h"
#include "./fragment_data/_all.h"
#include "./attachment_header.h"
#include "./attached_script.h"

namespace dovah::loaded_forms::components::papyrus {
   bool attachment_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (!this->header.load(subrecord))
         return false;
      uint16_t count;
      if (!subrecord.read(count))
         return false;
      this->scripts.reserve(this->scripts.size() + count);
      for (uint16_t i = 0; i < count; i++) {
         //
         // If a form contains multiple instances of the same script, the last-loaded one overrides 
         // the others in full; the previously loaded script data is cleared.
         //
         attached_script current;
         if (!current.load(this->header, subrecord))
            return false;
         if (auto* prior = this->lookup_script(current.name)) {
            *prior = current;
         } else {
            this->scripts.push_back(current);
         }
      }
      //
      if (subrecord.is_at_end()) // fragment data is optional
         return true;
      if (!subrecord.is_in_bounds())
         return false;
      switch (subrecord.containing_record_signature()) {
         case 'INFO':
            this->fragment_data = new topic_info_fragment_data;
            break;
         case 'PACK':
            this->fragment_data = new package_fragment_data;
            break;
         case 'PERK':
            this->fragment_data = new perk_fragment_data;
            break;
         case 'SCEN':
            this->fragment_data = new scene_fragment_data;
            break;
      }
      if (this->fragment_data)
         this->fragment_data->load(*this, subrecord);
      return subrecord.is_in_bounds();
   }
   bool attachment_data::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      this->header.save(subrecord);
      if (this->scripts.size() > std::numeric_limits<uint16_t>::max())
         return false;
      subrecord.write(uint16_t(this->scripts.size()));
      for (auto& script : this->scripts) {
         if (!script.save(this->header, subrecord, intfc))
            return false;
      }
      if (this->fragment_data)
         this->fragment_data->save(*this, subrecord);
      return true;
   }
   bool attachment_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (this->scripts.empty() && !this->fragment_data)
         return true;
      auto& VMAD = record.open_next_subrecord('VMAD');
      auto result = this->save(VMAD, intfc);
      VMAD.close();
      return result;
   }
   void attachment_data::clone_from(const attachment_data& other, loaded_forms::Form& owner_of_clone) noexcept {
      this->header = other.header;
      
      for (auto& script : this->scripts) { // clear any outbound use info we may have in here
         script.clear_properties(owner_of_clone);
      }
      this->scripts.clear();
      
      size_t size = other.scripts.size();
      this->scripts.resize(size);
      for (size_t i = 0; i < size; ++i)
         this->scripts[i].clone_from(other.scripts[i], owner_of_clone);
      //
      if (this->fragment_data) {
         this->fragment_data->clear(owner_of_clone);
         delete this->fragment_data;
         this->fragment_data = nullptr;
      }
      if (other.fragment_data)
         this->fragment_data = other.fragment_data->clone(owner_of_clone);
   }
   void attachment_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& script : this->scripts)
         script.sever_outbound_references_to(target, my_owner);
      if (this->fragment_data)
         this->fragment_data->sever_outbound_references_to(target, my_owner);
   }
   void attachment_data::clear(loaded_forms::Form& my_owner) noexcept {
      this->clear_scripts(my_owner);
      if (this->fragment_data)
         this->fragment_data->clear(my_owner);
   }

   void attachment_data::clear_scripts(loaded_forms::Form& my_owner) {
      for (auto& script : this->scripts)
         script.clear(my_owner);
   }

   /*static*/ attachment_header attachment_data::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t formID;
      //
      attachment_header header;
      uint16_t count;
      if (!header.load(subrecord))
         return header;
      if (!subrecord.read(count))
         return header;
      //
      // If a form contains multiple instances of the same script, the last-loaded one overrides 
      // the others in full; the previously loaded script data is cleared.
      //
      auto pos_before_scripts = subrecord.offset();
      std::unordered_map<std::string, uint32_t> script_counts;
      for (uint16_t i = 0; i < count; ++i) {
         std::string key;
         attached_script::extract_name_and_skip_remainder(header, subrecord, key);
         std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) -> unsigned char {
            if (c >= 'A' && c <= 'Z')
               return c + 0x20;
            return c;
         });
         ++script_counts[key];
      }
      subrecord.seek(pos_before_scripts); // can't just reset to the start of the subrecord; that breaks for scripts on aliases
      assert(subrecord.offset() == pos_before_scripts);
      //
      for (uint16_t i = 0; i < count; ++i) {
         std::string key;
         subrecord.read_length_prefixed_string<2>(key);
         std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) -> unsigned char {
            if (c >= 'A' && c <= 'Z')
               return c + 0x20;
            return c;
         });
         auto& remaining = script_counts[key];
         assert(remaining > 0);
         if (--remaining == 0) {
            attached_script::generate_use_info(header, subrecord, uib, true);
         } else {
            attached_script::skip_use_info(subrecord, true);
         }
      }
      //
      if (subrecord.is_at_end() || !subrecord.is_in_bounds()) // fragment data is optional
         return header;
      //
      // None of the fragment data types handled by the Papyrus loader can have use info. 
      // Quest fragments can, but those are handled in the Quest loader.
      //
      return header;
   }
   /*static*/ void attachment_data::skip_use_info(tes_subrecord_reader& subrecord) {
      attachment_header header;
      if (!header.load(subrecord))
         return;
      uint16_t count;
      if (!subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; ++i)
         attached_script::skip_use_info(subrecord, false);
   }

   /*static*/ void attachment_data::skim_vmad_for_scriptnames(tes_subrecord_reader& subrecord, std::vector<std::string>& out_attached, std::vector<std::string>& out_deleted) {
      out_attached.clear();
      out_deleted.clear();

      attachment_header header;
      if (!header.load(subrecord))
         return;

      uint16_t count;
      if (!subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; ++i) {
         std::string scriptname;
         subrecord.read_length_prefixed_string<2>(scriptname);
         //
         auto pos = subrecord.offset();
         //
         script_status status;
         subrecord.read(status);
         if (status == script_status::removed) {
            out_deleted.push_back(scriptname);
         } else {
            out_attached.push_back(scriptname);
         }
         //
         subrecord.seek(pos);
         attached_script::skip_use_info(subrecord, true);
      }
   }

   //
   
   void attachment_data::for_each_script(std::function<bool(attached_script&)> functor) {
      for (auto& item : this->scripts)
         if (functor(item))
            break;
   }
   void attachment_data::remove_script(loaded_forms::Form& owner, const std::string& name) {
      auto& list = this->scripts;
      for (auto it = list.begin(); it != list.end(); ++it) {
         auto& script = *it;
         if (dovah::papyrus::helpers::name_equals(script.name.c_str(), name.c_str()) != 0)
            continue;
         script.clear(owner);
         list.erase(it);
         return;
      }
   }
   void attachment_data::remove_script(loaded_forms::Form& owner, size_t index) {
      auto& list = this->scripts;
      if (index >= list.size())
         return;
      list[index].clear(owner);
      list.erase(list.begin() + index);
   }
}