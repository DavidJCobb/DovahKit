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
      static constexpr size_t bidi_worker_quartet_count = 2;

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

         class bidi_worker : public basic_reader {
            //
            // Given a form A which refers to a form B, and given a one-way outbound reference from A to B, we 
            // need to make that  reference bidirectional by  making B aware of the inbound  reference from A. 
            // This can basically only be done thread-safely when we have some way of pre-filtering every B in 
            // such a way that only one thread will access any given B a a time.
            // 
            // For the case of REFRs being reparented to new  exterior CELLs en masse, we can divide the CELLs 
            // up by their grid quadrant  -- that is, whether their grid X- and  Y-coordinates are positive or 
            // negative. We can have four threaded workers, one per quadrant, and use the coordinates as a way 
            // of discerning which worker each cell "belongs" to.
            //
            protected:
               load_order_persistent_ref_reparenter& owner;
               const size_t quartet_index = 0;
               const bool x_pos; // which quadrant should this worker handle? (true if positive on this axis)
               const bool y_pos; // which quadrant should this worker handle? (true if positive on this axis)
               std::thread thread;
               struct {
                  size_t maximum = 0;
                  size_t current = 0;
               } progress;
         
               static void _thread_handler(bidi_worker* instance) {
                  instance->_execute();
               }
               void _execute();

            public:
               bidi_worker(load_order_persistent_ref_reparenter&, size_t quartet_index, bool xp, bool yp);

               void start() noexcept;
               void wait_for() noexcept;
               
               inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
               float assess_load_progress() const noexcept;
         };
         struct bidi_worker_quartet {
            std::array<bidi_worker, 4> workers;

            bidi_worker_quartet(load_order_persistent_ref_reparenter& owner, size_t index) : workers{{
               { owner, index, true,  true },
               { owner, index, true,  false },
               { owner, index, false, true },
               { owner, index, false, false },
            }} {};

            inline void start() noexcept {
               for (auto& w : this->workers)
                  w.start();
            }
            inline void wait_for() noexcept {
               for (auto& w : this->workers)
                  w.wait_for();
            }
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
         std::array<bidi_worker_quartet, bidi_worker_quartet_count> bidi_workers;
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