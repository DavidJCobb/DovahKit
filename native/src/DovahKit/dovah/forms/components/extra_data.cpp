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
   extra_data_load_result extra_data_list::load(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      for (auto* extra : this->content) {
         auto result = extra->load(subrecord);
         if (result != load_result::unrecognized) {
            if (result == load_result::requires_record)
               result = extra->load(record) ? load_result::succeeded : load_result::failed;
            return result;
         }
      }
      auto data = create_extra_data_for_subrecord(subrecord);
      if (data) {
         this->content.push_back(data);
         auto result = data->load(subrecord);
         assert(result != load_result::unrecognized && "An extra-data class didn't recognize a subrecord that our factory definitions said was its responsibility.");
         if (result == load_result::requires_record)
            result = data->load(record) ? load_result::succeeded : load_result::failed;
         return result;
      }
      return load_result::unrecognized;
   }
   void extra_data_list::save(tes_record_writer& record) {
      for (auto* extra : this->content)
         extra->save(record);
   }
   /*static*/ extra_data_load_result extra_data_list::generate_use_info(tes_record_reader& record, form_stub* stub) {
      return generate_extra_data_use_info(record, stub);
   }
}