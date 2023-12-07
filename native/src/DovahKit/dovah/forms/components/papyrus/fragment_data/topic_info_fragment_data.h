#pragma once
#include <optional>
#include <string>
#include "./fragment_data_base.h"
#include "../basic_fragment.h"

namespace dovah::loaded_forms::components::papyrus {
   class topic_info_fragment_data : public fragment_data_base {
      public:
         static constexpr const fragment_type type = fragment_type::info;

      protected:
         struct fragment_flag {
            fragment_flag() = delete;
            enum type {
               has_begin_fragment = 0x01,
               has_end_fragment   = 0x02,
            };
         };
         
      public:
         topic_info_fragment_data() : fragment_data_base(type) {};

         virtual void load(attachment_data& owner, tes_subrecord_reader&) override;
         virtual void save(attachment_data& owner, tes_subrecord_writer&) override;
         virtual fragment_data_base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         
         uint8_t     unknown = 2;
         std::string filename;
         struct {
            std::optional<basic_fragment> on_begin;
            std::optional<basic_fragment> on_end;
         } fragments;
   };
}