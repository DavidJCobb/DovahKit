#pragma once
#include <limits>
#include "./types/class_array.h"
namespace dovah {
   namespace load_order_interfaces {
      class form_load;
      class form_save;
   }
   namespace loaded_forms {
      namespace components {
         class extra_data_use_info_state;
      }
      class Form;
   }
   namespace tes_file_reading {
      class record;
      class subrecord;
   }
   namespace tes_file_writing {
      class record;
      class subrecord;
   }
   class form_stub;
   class form_stub_use_info_builder;
}

namespace dovah::loaded_forms::components::extra_data_types {
   class extra_data {
      public:
         enum class subrecord_load_result {
            unrecognized,
            failed,
            succeeded,
            requires_record, // `extra_data::load` should return this if it needs to grab the immediate next subrecord(s)
         };
         enum class record_load_result {
            incomplete,
            complete,
         };

         using load_interface_t = load_order_interfaces::form_load;
         using save_interface_t = load_order_interfaces::form_save;

         using typecode_type = uint8_t;
         static_assert(std::numeric_limits<typecode_type>::max() - 1 >= all_extra_data_types::count);

      protected:
         constexpr extra_data(typecode_type t) : typecode(t) {}

      public:
         const typecode_type typecode;

      public:
         virtual subrecord_load_result load(tes_file_reading::subrecord& subrord, load_interface_t&) = 0;
         virtual record_load_result    load(tes_file_reading::record&, load_interface_t&) = 0;
         virtual void save(tes_file_writing::record&, save_interface_t&) = 0;
         
         static void generate_use_info(tes_file_reading::record&, form_stub_use_info_builder&, extra_data_use_info_state&) {}
         virtual void clear_contained_formIDs(loaded_forms::Form& my_owner) = 0;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) = 0;
         
         virtual extra_data* clone(loaded_forms::Form& clone_owner) const noexcept = 0;
   };
}