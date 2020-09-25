#include "extra_data.h"
#include "../_common_cpp.h"
#include <typeinfo>
#include "extra_data/_factory.h"

namespace dovah::loaded_forms::components {
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
}