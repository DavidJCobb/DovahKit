#pragma once
#include "../bare_form_id_t.h"

namespace dovah {
   class file_load_order;
   class form_stub;
}

namespace dovah {
   class form_renumber_request {
      friend class file_load_order;
      protected:
         file_load_order& owner;
         form_stub&       target;
         bare_form_id_t   desiredID = 0;
         
         form_renumber_request(file_load_order& o, form_stub& target);
         form_renumber_request(form_renumber_request&&);
         form_renumber_request(const form_renumber_request&) = delete;
         form_renumber_request& operator=(const form_renumber_request&) = delete;
         
      public:
         ~form_renumber_request();

         constexpr bare_form_id_t get_queued_form_id() const noexcept { return this->desiredID; }
         
         void commit();
   };
}