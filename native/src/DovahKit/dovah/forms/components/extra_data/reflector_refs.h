#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class reflector_refs : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XPWR';
         enum class type : uint32_t {
            reflection,
            refraction,
         };
         struct entry {
            form_reference_t target;
            type type;
         };
         //
         std::vector<entry> entries;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::lock; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_interface_t& intfc) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&, extra_data_use_info_state&);
         //
         virtual basic_extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
   };
}