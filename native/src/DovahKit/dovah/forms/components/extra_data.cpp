#include "extra_data.h"
#include "../_common_cpp.h"
#include <typeinfo>
#include "extra_data/_factory.h"

namespace dovah::loaded_forms::components {
   extra_data_list::~extra_data_list() {
      for (auto* extra : this->content)
         if (extra)
            delete extra;
      this->content.clear();
   }
   bool extra_data_list::insert(basic_extra_data* item) {
      auto& list = this->content;
      auto& type = typeid(*item);
      for (auto* extant : list) {
         if (type == typeid(*extant))
            return false;
      }
      list.push_back(item);
      return true;
   }
   void extra_data_list::remove(basic_extra_data* item) {
      auto& list = this->content;
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (*it == item) {
            list.erase(it);
            return;
         }
      }
   }
   void extra_data_list::remove_by_type(extra_data_type t) {
      auto& list = this->content;
      for (auto it = list.begin(); it != list.end(); ++it) {
         auto* item = *it;
         if (item->get_type() == t) {
            list.erase(it);
            return;
         }
      }
   }
   extra_data_load_result extra_data_list::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      for (auto* extra : this->content) {
         auto result = extra->load(subrecord, intfc);
         if (result != load_result::unrecognized) {
            if (result == load_result::requires_record)
               result = extra->load(record, intfc) ? load_result::succeeded : load_result::failed;
            return result;
         }
      }
      auto data = create_extra_data_for_subrecord(subrecord);
      if (data) {
         this->content.push_back(data);
         auto result = data->load(subrecord, intfc);
         assert(result != load_result::unrecognized && "An extra-data class didn't recognize a subrecord that our factory definitions said was its responsibility.");
         if (result == load_result::requires_record)
            result = data->load(record, intfc) ? load_result::succeeded : load_result::failed;
         return result;
      }
      return load_result::unrecognized;
   }
   void extra_data_list::save(tes_record_writer& record, save_interface_t& intfc) {
      for (auto* extra : this->content)
         extra->save(record, intfc);
   }
   void extra_data_list::clear(form_stub& my_owner) {
      for (auto* extra : this->content) {
         extra->clear_contained_formIDs(my_owner);
         delete extra;
      }
      this->content.clear();
   }
   void extra_data_list::clone_from(const extra_data_list& source, form_stub& my_owner) {
      for (auto* extra : this->content)
         extra->clear_contained_formIDs(my_owner);
      this->content.clear();
      //
      for (auto* extra : source.content) {
         auto* clone = extra->clone(my_owner);
         if (!clone) {
            #if _DEBUG
               __debugbreak();
            #endif
            continue;
         }
         this->content.push_back(clone);
      }
   }
   void extra_data_list::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      for (auto* extra : this->content)
         extra->sever_outbound_references_to(target, my_owner);
   }
   /*static*/ extra_data_load_result extra_data_list::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      return generate_extra_data_use_info(record, uib);
   }
   basic_extra_data* extra_data_list::lookup_by_type(extra_data_type t) const noexcept {
      for (auto* item : this->content)
         if (item && item->get_type() == t)
            return item;
      return nullptr;
   }
   basic_extra_data* extra_data_list::get_or_create_by_type(extra_data_type t) noexcept {
      auto* item = this->lookup_by_type(t);
      if (item)
         return item;
      item = create_extra_data_by_type(t);
      if (item)
         this->content.push_back(item);
      return item;
   }
}