#pragma once
#include <atomic>
#include <mutex>
#include <thread>
#include <type_traits>
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

      static constexpr size_t thread_count = 8;

      static constexpr bool multithreaded_bidirectional_use_info = true;

      class cell_map {
         public:
            static constexpr size_t grid_halfwidth  = 60;
            static constexpr size_t grid_total_size = (grid_halfwidth * 2) * (grid_halfwidth * 2);
         protected:
            using  cell_list = std::vector<form_stub*>;
            struct cell_entry {
               form_stub* cell = nullptr;
               std::atomic<size_t>     count = 0;
               std::vector<form_stub*> refs; // only writeable on main thread

               cell_entry() {}
               cell_entry(form_stub* c) : cell(c) {}
               cell_entry(const cell_entry& o) : cell(o.cell), count(o.count.load()), refs(o.refs) {}
               cell_entry(cell_entry&& o) {
                  this->count = o.count.load();
                  this->cell  = o.cell;
                  std::swap(this->refs, o.refs);
               }

               cell_entry& operator=(const cell_entry& o) {
                  this->count = o.count.load();
                  this->cell  = o.cell;
                  this->refs  = o.refs;
                  return *this;
               }
               cell_entry& operator=(cell_entry&& o) noexcept {
                  this->count = o.count.load();
                  this->cell  = o.cell;
                  std::swap(this->refs, o.refs);
                  return *this;
               }
            };
         public:
            form_stub* world = nullptr;
            std::vector<cell_entry> unsorted;
            std::array<cell_entry, grid_total_size> sorted;
            
            form_stub* cell_for_position(float x, float y); // calling this also increases the relevant cell_entry's count
            void set_world(form_stub& world);
            void clear();

            void take_ref(form_stub* refr, int32_t gx, int32_t gy); // main thread only
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
               void start();
               void wait_for();
               
               inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
               float assess_load_progress() const noexcept;
         };

         class bidi_worker {
            protected:
               load_order_persistent_ref_reparenter& owner;
               std::thread thread;
               struct {
                  size_t start = 0;
                  size_t end   = 0;
               } range;

               static void _thread_handler(bidi_worker* instance) {
                  instance->_execute();
               }
               void _execute();
            public:
               bidi_worker(load_order_persistent_ref_reparenter&);

               void set_range(size_t start, size_t end);
               void start();
               void wait_for();

               inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         };

         struct reference {
            form_stub* refr   = nullptr; // persistent REFR
            form_stub* cell   = nullptr; // cell we've reparented the REFR to
            int32_t    grid_x = 0;
            int32_t    grid_y = 0;

            reference() {}
            reference(form_stub* r) : refr(r) {}
         };

      protected:
         load_order_persistent_ref_reparenter();

         std::mutex lock;
         std::array<worker,      thread_count> threads;
         std::array<bidi_worker, thread_count> bidi_workers;
         std::vector<reference> refs;
         cell_map cells_for_current_world;

         void clear();

      public:
         static load_order_persistent_ref_reparenter& get() {
            static load_order_persistent_ref_reparenter instance;
            return instance;
         }

         void execute(file_load_order&);
   };
}