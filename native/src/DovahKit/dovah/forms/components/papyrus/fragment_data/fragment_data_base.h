#pragma once
#include <cstdint>
#include "../_forward_declare_file_handling.h"

namespace dovah {
   namespace loaded_forms {
      namespace components::papyrus {
         class attachment_data;
      }
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::components::papyrus {
   enum class fragment_type : uint8_t {
      undefined,
      info,
      package,
      perk,
      scene,
   };

   class fragment_data_base {
      public:
         const fragment_type type;
         
         fragment_data_base(fragment_type t) : type(t) {}
         
         virtual void load(attachment_data& owner, tes_subrecord_reader&, load_order_interfaces::form_load&) = 0;
         virtual void save(attachment_data& owner, tes_subrecord_writer&, load_order_interfaces::form_save&) = 0;
         virtual fragment_data_base* clone(loaded_forms::Form& owner_of_clone) const noexcept = 0;
         virtual void clear(loaded_forms::Form& owner) {}
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {}
   };
}