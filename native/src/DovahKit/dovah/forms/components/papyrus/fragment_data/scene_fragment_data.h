#pragma once
#include <limits>
#include <optional>
#include <string>
#include <vector>
#include "./fragment_data_base.h"
#include "../basic_fragment.h"

namespace dovah::loaded_forms::components::papyrus {
   class scene_fragment_data : public fragment_data_base {
      public:
         static constexpr const fragment_type type = fragment_type::scene;

      protected:
         struct fragment_flag {
            fragment_flag() = delete;
            enum type {
               has_begin_fragment = 0x01,
               has_end_fragment   = 0x02,
            };
         };

      protected:
         using phase_fragment_count_serialized_type = uint16_t;
      public:
         static constexpr const size_t max_phase_fragment_count = std::numeric_limits<phase_fragment_count_serialized_type>::max();

         struct phase_fragment {
            struct flag {
               flag() = delete;
               enum {
                  on_start      = 1,
                  on_completion = 2,
               };
            };

            uint8_t     flags = 0;
            uint32_t    phase; // zero-indexed internally; one-indexed in UI. if no flags set, it's actually an action ID
            uint8_t     unknown05;
            std::string filename;
            std::string function;
         };
         
      public:
         scene_fragment_data() : fragment_data_base(type) {};

         virtual void load(attachment_data& owner, tes_subrecord_reader&) override;
         virtual void save(attachment_data& owner, tes_subrecord_writer&, load_order_interfaces::form_save&) override;
         virtual fragment_data_base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         
         uint8_t     unknown = 2;
         std::string filename;
         struct {
            std::optional<basic_fragment> on_begin;
            std::optional<basic_fragment> on_end;
            //
            std::vector<phase_fragment> on_phase; // max count is `max_phase_fragment_count`
         } fragments;
   };
}