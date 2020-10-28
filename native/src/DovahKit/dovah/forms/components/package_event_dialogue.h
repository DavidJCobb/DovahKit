#pragma once
#include <array>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct package_event_dialogue { // TESPackage::Data
      enum class topic_type : uint32_t {
         ref     = 0,
         subtype = 1,
      };
      //
      form_reference_t idle;
      topic_type       type;
      form_reference_t topic;             // this and the next field are union'd in the file based on (type)
      uint32_t         topic_subtype = 0; // signature
      //
      bool load(tes_record_reader&, load_order_interfaces::form_load& intfc);
      static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      void save(tes_record_writer&);
      void clone_from(const package_event_dialogue& original, form_stub& owner_of_clone) noexcept;
      void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;

      bool empty() const noexcept;
   };
}