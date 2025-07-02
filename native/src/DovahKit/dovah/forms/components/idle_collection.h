#pragma once
#include <cstdint>
#include "../_common.h"

namespace dovah {
   namespace loaded_forms::components {
      class idle_collection { // BGSIdleCollection
         public:
            static constexpr const uint32_t subrecord_signature_array = 'IDLA';
            static constexpr const uint32_t subrecord_signature_count = 'IDLC';
            static constexpr const uint32_t subrecord_signature_flags = 'IDLF';
            static constexpr const uint32_t subrecord_signature_timer = 'IDLT';

            static constexpr const size_t max_idles_count = std::numeric_limits<uint8_t>::max();
            
            struct flag {
               enum type : uint8_t {
                  run_in_sequence    = 1 << 0,
                  do_once            = 1 << 2,
                  ignored_by_sandbox = 1 << 4,
               };
            };
            using flags_t = std::underlying_type_t<flag::type>;

         protected:
            struct {
               uint8_t count = 0;
            } _load_state;
         public:
            flags_t  flags = 0;
            float    timer = 0;
            std::vector<form_reference_t> idles;

            void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
            void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
            void clone_from(const idle_collection& original, loaded_forms::Form& owner_of_clone) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            void clear(loaded_forms::Form& my_owner);
         
            struct use_info_state {
               std::vector<form_id_t> idles;
               uint8_t count = 0;
               
               void read(tes_subrecord_reader&);
               void commit(form_stub_use_info_builder&);
            };
      };
   }
}