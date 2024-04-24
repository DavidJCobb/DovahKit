#pragma once
#include <limits>
#include <string>
#include <vector>
#include "./fragment_data_base.h"
#include "../basic_fragment.h"

namespace dovah::loaded_forms::components::papyrus {
   class perk_fragment_data : public fragment_data_base {
      public:
         static constexpr const fragment_type type = fragment_type::perk;

      protected:
         using fragment_count_serialized_type = uint16_t;
      public:
         static constexpr const size_t max_fragment_count = std::numeric_limits<fragment_count_serialized_type>::max();

         struct fragment {
            uint16_t index;
            uint16_t unknown02;
            uint8_t  unknown04;
            std::string filename;
            std::string function;
         };
         
      public:
         perk_fragment_data() : fragment_data_base(type) {};

         virtual void load(attachment_data& owner, tes_subrecord_reader&) override;
         virtual void save(attachment_data& owner, tes_subrecord_writer&, load_order_interfaces::form_save&) override;
         virtual fragment_data_base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         
         uint8_t     unknown = 2;
         std::string filename;
         std::vector<fragment> fragments; // max count is `max_fragment_count`
   };
}