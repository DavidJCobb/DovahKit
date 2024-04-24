#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "./_forward_declare_file_handling.h"
#include "./attachment_header.h" // for whatever reason, forward-declaring this makes the linker choke and die
#include "./property.h"
#include "./script_status.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::components::papyrus {
   class attached_script {
      public:
         static constexpr const size_t max_property_count = (1 << 29) - 1;

      public:
         std::string   name;
         script_status status = script_status::defined_locally;
         std::vector<property> properties;
                  
         bool load(const attachment_header& owner, tes_subrecord_reader&);
         void save(const attachment_header& owner, tes_subrecord_writer&, load_order_interfaces::form_save&) noexcept;

         void clear_properties(loaded_forms::Form& owner);
         void clone_properties(loaded_forms::Form& my_owner, const attached_script& other);

         void clone_from(const attached_script& source, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner) noexcept;
                  
         static void extract_name_and_skip_remainder(const attachment_header& header, tes_subrecord_reader&, std::string&);
         static void generate_use_info(const attachment_header& header, tes_subrecord_reader&, form_stub_use_info_builder&, bool already_read_name);
         static void skip_use_info(tes_subrecord_reader&, bool already_read_name);

         constexpr bool name_matches(const std::string_view& v) const;
         constexpr bool name_matches(const std::string& v) const;

         constexpr const property* lookup_property(const std::string_view& name) const;
         constexpr const property* lookup_property(const std::string& name) const;
         constexpr property* lookup_property(const std::string_view& name);
         constexpr property* lookup_property(const std::string& name);

         void remove_property(loaded_forms::Form& owner, const std::string& name);
         void remove_property(loaded_forms::Form& owner, size_t index);

         //
         // NOTE:
         //
         //  - If we define an operator= that affects use info, then we'll break script_data::load unless 
         //    and until we change it to pre-extract script names and load only the last script with each 
         //    name (akin to what's done for use info). Ditto for if we define a destructor that tries to 
         //    sever use info. Fortunately, it's not currently possible to do that, since form components 
         //    aren't aware of their containing forms and there's no way to pass that information into an 
         //    assignment statement or destructor; however,  if the situation changes, we should remember 
         //    these limits.
         //
   };
}

#include "./attached_script.inl"