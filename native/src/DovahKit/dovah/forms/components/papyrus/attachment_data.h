#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include "./_forward_declare_file_handling.h"
#include "./attachment_header.h"
#include "./attached_script.h"

namespace dovah {
   namespace loaded_forms {
      namespace components::papyrus {
         class fragment_data_base;
      }
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::components::papyrus {
   class attachment_data {
      public:
         attachment_header   header;
         std::vector<attached_script> scripts;
         fragment_data_base* fragment_data = nullptr;
         
         bool load(tes_subrecord_reader&, load_order_interfaces::form_load&); // assumes we're at a VMAD subrecord
         bool save(tes_subrecord_writer&, load_order_interfaces::form_save&);
         static attachment_header generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         static void skip_use_info(tes_subrecord_reader&);

         static void skim_vmad_for_scriptnames(tes_subrecord_reader&, std::vector<std::string>& out_attached, std::vector<std::string>& out_deleted);
         
         bool save(tes_record_writer&, load_order_interfaces::form_save&); // opens VMAD, writes, closes; doesn't write a subrecord if there are no scripts attached
         void clone_from(const attachment_data& source, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner) noexcept;

         void clear_scripts(loaded_forms::Form& my_owner);
         
         constexpr bool empty() const noexcept { return this->scripts.empty(); }

         void for_each_script(std::function<bool(attached_script&)>); // return true to stop iterating early

         constexpr const attached_script* lookup_script(const std::string_view& name) const;
         constexpr const attached_script* lookup_script(const std::string& name) const;
         constexpr attached_script* lookup_script(const std::string_view& name);
         constexpr attached_script* lookup_script(const std::string& name);

         void remove_script(loaded_forms::Form& owner, const std::string& name);
         void remove_script(loaded_forms::Form& owner, size_t index);
   };
}

#include "./attachment_data.inl"