#pragma once
#include <array>
#include "./process_params.h"
#include "./threads/exterior_cells_and_children.h"
#include "./threads/forms_of_type.h"
#include "./threads/game_settings.h"
#include "./threads/interior_cells_and_children.h"
#include "./threads/persistent_cell_children.h"
#include "./threads/topics_and_infos.h"
namespace dovah {
   namespace load_order_processes::build_form_stubs_from_file::threads {
      namespace impl {
         class base_implementation;
      }
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
         friend threads::impl::base_implementation;
         private:
            constexpr accept_stub() {}
      };
   }

   class process {
      protected:
         struct thread_collection {
            public:
               std::array<threads::exterior_cells_and_children, 6> exterior_cells_and_children;
               std::array<threads::forms_of_type,               4> forms_of_type;
               std::array<threads::game_settings,               1> game_settings;
               std::array<threads::interior_cells_and_children, 4> interior_cells_and_children;
               std::array<threads::persistent_cell_children,    2> persistent_cell_children;
               std::array<threads::topics_and_infos,            1> topics_and_infos;

            public:
               thread_collection(const process_params&, process&);

               void start();
               void wait();
               void rethrow_earliest_caught_exception(); // does nothing if no exception was caught by any thread
         };

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
         process_params    _params;
         thread_collection _threads;
         const bool _source_file_is_active;
   };
}