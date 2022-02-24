#pragma once
#include <mutex>
#include <thread>
#include "basic_reader.h"
#include "helpers/grid.h"
#include "helpers/singleton.h"

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

      static constexpr size_t thread_count = 4;

      static constexpr uint8_t max_pre_sort_world_grid = 60; // When prefetching cells, sort all that fall within (+/-) this size into a two-dimensional array for faster lookups.
      class world_cell_map {
         protected:
            using cell_list = std::vector<form_stub*>;
            using cell_grid = cobb::centered_square_grid<form_stub*, max_pre_sort_world_grid>;
         public:
            form_stub* world = nullptr;
            cell_list  unsorted;
            cell_grid  sorted;
            
            form_stub* cell_for_position(float x, float y) const;
            void set_world(form_stub& world);

            void clear();
      };

      protected:
         class worker : public basic_reader {
            protected:
               load_order_persistent_ref_reparenter& owner;
               std::thread thread;
               struct {
                  size_t start = 0;
                  size_t end   = 0;
               } range;
               struct {
                  uint32_t maximum = 0;
                  uint32_t current = 0;
               } progress;
         
               static void _thread_handler(worker* instance) {
                  instance->_execute();
               }
               void _execute();

            public:
               worker(load_order_persistent_ref_reparenter&);

               void set_range(size_t start, size_t end);
               void start() noexcept;
               void wait_for() noexcept;
               
               inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
               float assess_load_progress() const noexcept;
         };

         struct ref_entry {
            form_stub* refr = nullptr;
            form_stub* cell = nullptr;
            int32_t    gx   = 0;
            int32_t    gy   = 0;

            ref_entry() {}
            ref_entry(form_stub& refr) : refr(&refr) {}
         };

      protected:
         load_order_persistent_ref_reparenter();

         std::mutex lock;
         std::array<worker, thread_count> threads;
         world_cell_map cells_for_current_world;
         std::vector<ref_entry> refs;

      public:
         static load_order_persistent_ref_reparenter& get() {
            static load_order_persistent_ref_reparenter instance;
            return instance;
         }

         void execute(file_load_order&);
   };
}