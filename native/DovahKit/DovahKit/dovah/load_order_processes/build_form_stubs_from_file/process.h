#pragma once
#include <array>
#include "./process_params.h"
#include "./threads/forms_of_type.h"
namespace dovah {
   namespace load_order_processes::build_form_stubs_from_file::threads {
      template<typename Self>
      class _base;
   }
   namespace tes {
      class  file;
      class  record_view;
      struct record_header;
   }
   class active_load_order;
   class form_stub;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   namespace passkeys {
      class accept_stub {
         template<typename> friend class threads::_base;
         private:
            constexpr accept_stub() {}
      };
   }

   class process {
      template<typename> friend class threads::_base;
      public:
         process(const process_params& p);

         // May throw.
         void execute();

         void abort();

      public: // passkeyed
         void _done_parsing_stub(
            passkeys::accept_stub,
            form_stub*&,
            tes::record_view&
         );

      protected:
         void _commit_stub(
            form_stub*&,
            size_t record_offset,
            const tes::record_header&
         );

      protected:
         process_params _params;
         struct thread_collection {
            std::array<threads::forms_of_type, 4> forms_of_type;
         } _threads;
   };
}