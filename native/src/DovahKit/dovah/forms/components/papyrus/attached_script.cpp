#include "./attached_script.h"
#include "../../_common_cpp.h"
#include "../../../logging.h"
#include "./attachment_header.h"

namespace dovah::loaded_forms::components::papyrus {
   bool attached_script::load(const attachment_header& header, tes_subrecord_reader& subrecord) {
      subrecord.read_length_prefixed_string<2>(this->name);
      uint16_t count;
      if (!subrecord.is_in_bounds(sizeof(this->status) + sizeof(count)))
         return false;
      subrecord.unchecked_read(this->status);
      subrecord.unchecked_read(count);
      this->properties.reserve(this->properties.size() + count);
      for (uint16_t i = 0; i < count; i++) {
         property prop;
         if (!prop.load(header, subrecord))
            return false;
         if (auto* prior = this->lookup_property(prop.name)) {
            *prior = prop;
         } else {
            this->properties.push_back(prop);
         }
      }
      return true;
   }
   bool attached_script::save(const attachment_header& header, tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) noexcept {
      subrecord.write_length_prefixed_string<2>(this->name);
      uint16_t count = this->properties.size();
      if (this->properties.size() > std::numeric_limits<decltype(count)>::max()) {
         dovah::logging::print_line("Problem encountered while saving script %s: too many properties.", this->name.c_str());
         return false;
      }
      subrecord.write(this->status);
      subrecord.write(count);
      for (uint16_t i = 0; i < count; i++) {
         auto& prop = this->properties[i];
         if (!prop.save(header, subrecord, intfc)) {
            dovah::logging::print_line("Problem encountered while saving property %d for script %s.", i, this->name.c_str());
            return false;
         }
      }
      return true;
   }

   void attached_script::clear_properties(loaded_forms::Form& my_owner) {
      for (auto& prop : this->properties)
         prop.clear(my_owner);
      this->properties.clear();
   }
   void attached_script::clone_properties(loaded_forms::Form& my_owner, const attached_script& other) {
      auto& a = this->properties;
      auto& b = other.properties;
      a.reserve(a.size() + b.size());
      for (auto& p : other.properties) {
         auto& copy = a.emplace_back();
         copy.clone_from(p, my_owner);
      }
   }

   void attached_script::clone_from(const attached_script& other, loaded_forms::Form& owner_of_clone) noexcept {
      this->name   = other.name;
      this->status = other.status;
      //
      size_t size = other.properties.size();
      this->clear_properties(owner_of_clone);
      this->properties.resize(size);
      for (size_t i = 0; i < size; ++i) {
         this->properties[i].clone_from(other.properties[i], owner_of_clone);
      }
   }
   void attached_script::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& prop : this->properties)
         prop.sever_outbound_references_to(target, my_owner);
   }
   void attached_script::clear(loaded_forms::Form& my_owner) noexcept {
      this->clear_properties(my_owner);
   }

   //
   
   /*static*/ void attached_script::extract_name_and_skip_remainder(const attachment_header& header, tes_subrecord_reader& subrecord, std::string& out) {
      subrecord.read_length_prefixed_string<2>(out);
      skip_use_info(subrecord, true);
   }
   /*static*/ void attached_script::generate_use_info(const attachment_header& header, tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib, bool already_read_name) {
      form_id_t formID;
      //
      if (!already_read_name) {
         subrecord.skip_length_prefixed_string<2>();
      }
      subrecord.skip_bytes(1); // script status
      uint16_t prop_count;
      if (!subrecord.read(prop_count))
         return;
      //
      // If a script contains multiple instances of the same property, the last-loaded one 
      // overrides the others in full.
      //
      auto pos_before_props = subrecord.offset();
      std::unordered_map<std::string, uint32_t> prop_counts;
      for (uint16_t i = 0; i < prop_count; ++i) {
         std::string key;
         property::extract_name_and_skip_remainder(header, subrecord, key);
         std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
         ++prop_counts[key];
      }
      subrecord.seek(pos_before_props); // can't just reset to the start of the subrecord; that breaks for scripts on aliases
      assert(subrecord.offset() == pos_before_props);
      //
      for (uint16_t i = 0; i < prop_count; ++i) {
         std::string key;
         subrecord.read_length_prefixed_string<2>(key);
         std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
         auto& remaining = prop_counts[key];
         assert(remaining > 0);
         if (--remaining == 0) {
            property::generate_use_info(header, subrecord, uib, true);
         } else {
            property::skip_use_info(subrecord, true);
         }
      }
   }
   /*static*/ void attached_script::skip_use_info(tes_subrecord_reader& subrecord, bool already_read_name) {
      form_id_t formID;
      //
      if (!already_read_name) {
         subrecord.skip_length_prefixed_string<2>();
      }
      subrecord.skip_bytes(1); // script status
      uint16_t prop_count;
      if (!subrecord.read(prop_count))
         return;
      for (uint16_t j = 0; j < prop_count; j++)
         property::skip_use_info(subrecord, false);
   }

   //
   
   property* attached_script::lookup_property(const std::string& name) {
      auto& list = this->properties;
      for (auto& prop : list)
         if (_stricmp(prop.name.c_str(), name.c_str()) == 0)
            return &prop;
      return nullptr;
   }
   void attached_script::remove_property(loaded_forms::Form& owner, const std::string& name) {
      auto& list = this->properties;
      for (auto it = list.begin(); it != list.end(); ++it) {
         auto& prop = *it;
         if (_stricmp(prop.name.c_str(), name.c_str()) != 0)
            continue;
         prop.clear(owner);
         list.erase(it);
         return;
      }
   }
   void attached_script::remove_property(loaded_forms::Form& owner, size_t index) {
      auto& list = this->properties;
      if (index >= list.size())
         return;
      list[index].clear(owner);
      list.erase(list.begin() + index);
   }
}