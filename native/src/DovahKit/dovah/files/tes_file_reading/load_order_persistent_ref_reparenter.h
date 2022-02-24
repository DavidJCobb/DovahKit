#pragma once
#include <limits>
#include <mutex>
#include <thread>
#include <type_traits>
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

      static constexpr size_t thread_count = 4;

      // For each worldspace, gather relevant child cells into a std::vector before working, for faster searches.
      static constexpr bool pre_list_world_cells = true;
      // When pre-sorting cells, sort all that fall within (+/-) this size into a two-dimensional array for faster lookups.
      static constexpr uint8_t max_pre_sort_world_grid = pre_list_world_cells ? 30 : 0;
      class cached_cell_list {
         protected:
            using cell_list = std::vector<form_stub*>;
            struct dummy {};

            struct by_grid {
               struct range {
                  using value_type = int8_t;
                  value_type x = 0;
                  value_type y = 0;
                  //
                  template<typename T> requires std::is_arithmetic_v<T> static bool can_represent(T v) {
                     using limits = std::numeric_limits<value_type>;
                     return v >= std::min(-(int32_t)max_pre_sort_world_grid, (int32_t)limits::min()) && v <= std::min((int32_t)max_pre_sort_world_grid, (int32_t)limits::max());
                  }
               };

               cell_list data;
               struct {
                  alignas(range::value_type) range min;
                  alignas(range::value_type) range max;
               } bounds;

               form_stub* lookup(int32_t gx, int32_t gy) const;
            };
            using sorted_cells   = std::conditional_t<(pre_list_world_cells && max_pre_sort_world_grid > 0), by_grid, dummy>;
            using unsorted_cells = std::conditional_t<pre_list_world_cells, cell_list, dummy>;

         public:
            form_stub*     world = nullptr;
            sorted_cells   sorted;
            unsorted_cells unsorted;
            //
            form_stub* cell_for_position(float x, float y) const;
            void set_world(form_stub& world);
      };

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

         std::mutex lock;
         std::array<worker, thread_count> threads = {};
         cached_cell_list cells_for_current_world;

      public:
         static load_order_persistent_ref_reparenter& get() {
            static load_order_persistent_ref_reparenter instance;
            return instance;
         }

         void execute(file_load_order&);
   };
}