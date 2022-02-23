#pragma once
#include <mutex>
#include <thread>
#include <unordered_map>
#include "basic_reader.h"
#include "../../../helpers/singleton.h"

//
// All of a worldspace's persistent references are encoded as children of the persistent 
// cell. However, this is cumbersome for frontends to work with, and what's more, it's not 
// necessary within the backend: when saving, we already handle the case of persistent refs 
// existing in non-persistent cells.
// 
// Accordingly, after forms are loaded and before use info is built, we want to reparent 
// the children of persistent cells whenever it is possible to do so -- that is, whenever 
// these children have a defined position and whenever that position places them isnide of 
// an existing cell in their worldspace.
//

namespace dovah {
   class form_stub;
   class file_load_order;
   namespace tes_file_reading {
      class record;
   }
   namespace load_order_interfaces {
      class form_load;
   }
}

namespace dovah::tes_file_reading {
   class load_order_persistent_ref_reparenter : public cobb::singleton {
      using coord_value_t = int32_t;
      using coordinates_t = std::pair<coord_value_t, coord_value_t>;

      using cached_cell_list = std::vector<form_stub*>;

      static constexpr size_t thread_count = 4;

      protected:
         class worker : public basic_reader {
            protected:
               form_stub* world = nullptr;
               std::vector<form_stub*> queue;
               std::thread thread;
               struct {
                  uint32_t maximum = 0;
                  uint32_t current = 0;
               } progress;
               cached_cell_list* cells = nullptr;
         
               static void _thread_handler(worker* instance) {
                  instance->_execute();
               }
               void _execute();

            public:
               worker();

               void set_world(form_stub& world) noexcept;
               void set_cached_cell_list(cached_cell_list&);
               void add_to_queue(form_stub& stub) noexcept;
               void start() noexcept;
               void wait_for() noexcept;
               
               inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
               float assess_load_progress() const noexcept;

               inline const std::vector<form_stub*>& view_queue() const noexcept { return this->queue; }
         };

      protected:
         load_order_persistent_ref_reparenter();

         std::unordered_map<form_stub*, coordinates_t> positions;
         std::mutex lock;
         std::array<worker, thread_count> threads = {};
         cached_cell_list cells_for_current_world;

         void _receive(form_stub*, coordinates_t);

      public:
         static load_order_persistent_ref_reparenter& get() {
            static load_order_persistent_ref_reparenter instance;
            return instance;
         }

         void execute(file_load_order&);
   };
}