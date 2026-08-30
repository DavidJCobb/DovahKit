#include "./extra_data_list.h"
#include <cassert>
#include <utility> // std::unreachable
#include "../../_common_cpp.h"
#include "./extra_data.h"
#include "./factories/construct_for_subrecord.h"
#include "./factories/generate_use_info.h"
#include "./utils/extra_data_type_has_post_load_validation.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components {
   extra_data_list::~extra_data_list() {
      for (auto* extra : this->content)
         if (extra)
            delete extra;
      this->content.clear();
   }
   bool extra_data_list::insert(extra_data* item) {
      auto& list = this->content;
      for (auto* extant : list) {
         if (extant->typecode == item->typecode)
            return false;
      }
      list.push_back(item);
      return true;
   }
   void extra_data_list::remove(extra_data* item) {
      auto& list = this->content;
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (*it == item) {
            list.erase(it);
            return;
         }
      }
   }

   extra_data_list::load_result extra_data_list::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      for (auto* extra : this->content) {
         auto result = extra->load(subrecord, intfc);
         if (result != subrecord_load_result::unrecognized) {
            if (result == subrecord_load_result::requires_record)
               extra->load(record, intfc);
            return load_result::succeeded;
         }
      }
      auto* data = extra_data_factories::construct_for_subrecord(subrecord.signature());
      if (data) {
         this->content.push_back(data);
         auto result = data->load(subrecord, intfc);
         #if _DEBUG
            if (result == subrecord_load_result::unrecognized)
               //
               // An extra-data class didn't recognize a subrecord that our factory definitions said was its 
               // responsibility. This could happen if the record data is ill-formed, e.g. if an extra-data 
               // type consists of multiple subrecords with strict ordering and a leading subrecord is not 
               // present; however, it could as easily be a mistake on our part.
               //
               __debugbreak();
         #endif
         switch (result) {
            case subrecord_load_result::failed:
            case subrecord_load_result::unrecognized:
               return load_result::failed;
            case subrecord_load_result::succeeded:
               return load_result::succeeded;
            case subrecord_load_result::requires_record:
               data->load(record, intfc);
               return load_result::succeeded;
         }
         std::unreachable();
      }
      return load_result::unrecognized;
   }
   void extra_data_list::post_load_validation(load_order_interfaces::form_load& intfc) {
      for (auto* extra : this->content) {
         if (!extra_data_utils::extra_data_has_post_load_validation(*extra))
            continue;
         extra->post_load_validation(intfc);
      }
   }
   void extra_data_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      for (auto* extra : this->content)
         extra->save(record, intfc);
   }
   void extra_data_list::clear(loaded_forms::Form& my_owner) {
      for (auto* extra : this->content) {
         extra->clear_contained_formIDs(my_owner);
         delete extra;
      }
      this->content.clear();
   }
   void extra_data_list::clone_from(const extra_data_list& source, loaded_forms::Form& my_owner) {
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
   void extra_data_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto* extra : this->content)
         extra->sever_outbound_references_to(target, my_owner);
   }
   /*static*/ bool extra_data_list::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      return extra_data_factories::generate_use_info(record, uib, state);
   }
}