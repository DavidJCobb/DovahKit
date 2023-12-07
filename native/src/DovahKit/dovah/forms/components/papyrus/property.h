#pragma once
#include "./_forward_declare_file_handling.h"
#include "./property_type.h"
#include "./property_value.h"

namespace dovah::loaded_forms::components::papyrus {
   class property {
      public:
         struct status_flag {
            status_flag() = delete;
            enum type : uint8_t {
               altered   = 1, // property has been set on the form to which this ScriptObject is attached (i.e. script and property on base form, or script on base form and property on REFR)
               inherited = 2, // used for a Papyrus property on a REFR, when you attach a script to a base form and then further edit its properties on a REFR
               removed   = 3, // the CK expects this to only ever occur alongside `inherited`
               //
               // The CK shows "<unknown>" for combinations of flags besides 0b01, 0b10, and 0b11. The `removed` flag 
               // is never supposed to appear on its own.
               //
            };
         };
         using status_flags = status_flag::type;

      public:
         std::string    name;
         status_flags   status = (status_flags)0;
         property_value value;

         constexpr property_type type() const noexcept { return property_type_for(this->value); }
         //
         constexpr bool is_array() const noexcept { return property_type_is_array(this->type()); }
         constexpr property_type scalar_type() const noexcept { return scalar_property_type_for(this->type()); }
                  
         bool load(const attachment_header& header, tes_subrecord_reader&);
         bool save(const attachment_header& header, tes_subrecord_writer&, load_order_interfaces::form_save&) noexcept;
         void clone_from(const property& source, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner) noexcept;

         // Changes the type of the property while properly clearing its value beforehand, to ensure that use info is properly managed.
         void set_type(loaded_forms::Form& my_owner, property_type);
                  
         static void extract_name_and_skip_remainder(const attachment_header& header, tes_subrecord_reader&, std::string&);
         static void generate_use_info(const attachment_header& header, tes_subrecord_reader&, form_stub_use_info_builder&, bool already_read_name);
         static void skip_use_info(tes_subrecord_reader&, bool already_read_name);
   };
}