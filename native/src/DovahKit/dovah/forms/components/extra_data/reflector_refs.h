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
            form_id_t target;
            type      type;
         };
         //
         std::vector<entry> entries;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::lock; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
         virtual void get_outbound_formIDs(std::vector<form_id_t*>&) const noexcept override;
         virtual void on_after_delete() noexcept override;
         virtual bool is_empty() const noexcept override;
         //
         void collapse() noexcept;
   };
}