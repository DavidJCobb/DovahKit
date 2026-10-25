#include "./worker_thread.h"
#include <cassert>
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/active_load_order.h"
#include "dovah/form_data_function_tables/build_outbound_use_info.h"
#include "dovah/form_stub.h"
#include "dovah/form_type_info.h"
#include "dovah/termination_handler.h"
#include "./build_hardcoded_uses.h"
#include "./use_info_builder.h"

namespace dovah::load_order_processes::use_info_build {
   void worker_thread::_execute() {
      termination_handler::update_this_thread();

      const auto* active_file = this->owner.get_active_file();

      auto& list = this->queue;
      this->progress.maximum = (uint32_t)list.size();
      for (auto* stub : list) {
         const auto& fti = form_type_info::lookup(stub->form_type);
         if (fti.flags & form_type_info::flag::no_connections)
            //
            // Don't waste time walking records for form types that can never have outbound uses.
            //
            continue;

         const bool can_be_parent = fti.flags & form_type_info::flag::can_have_children;
         const auto file_count    = stub->source_file_count();

         use_info_builder uib(*stub);
         for (uint16_t i = 0; i < file_count; ++i) {
            const auto* source_info = stub->get_source_file_info(i);
            assert(!!source_info);
            const auto* source_file = source_info->pointer;
            assert(!!source_file);
            uib._is_active_file    = (source_file == active_file);
            uib._is_base_record    = (i == 0);
            uib._is_final_file     = (i == file_count - 1);
            uib._is_partial_record = false;
            if (i == 0) {
               if (source_file->app_flags().has_all_of<tes::file_app_specific_flag::is_hardcoded_dummy>()) {
                  build_hardcoded_uses(uib);
                  uib.commit();
                  continue;
               }
            }
            if (!source_info->record_offset)
               continue;

            auto view = tes::file_view::open_view_to_record(*source_file, source_info->record_offset);
            if (i > 0 && can_be_parent && source_info->record_flags & (1 << ((uint32_t)tes::record_flag::partial))) {
               uib._is_partial_record = true;
            }
            auto func = form_data_function_tables::build_outbound_use_info::lookup(stub->form_type);
            if (func) {
               auto& record = view.subviews.record;
               (func)(record, uib);
               uib.commit();

               bool didnt_read_anything = record.position() == 0 && !record.get_current_subrecord();
               bool read_everything     = !record.is_in_bounds();

               assert((didnt_read_anything || read_everything) && "A form type's `build_outbound_use_info` function should either read nothing (not even opening any subrecords), or read the whole record! (Reading nothing is fine if the form type in question can't refer to any other forms.)");
            }
         }
         ++this->progress.current;
      }
      list.clear();
   }

   void worker_thread::add_to_queue(form_stub* stub) noexcept {
      this->queue.push_back(stub);
   }
   void worker_thread::start() noexcept {
      this->thread = std::thread(worker_thread::_thread_handler, this);
   }
   void worker_thread::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
   float worker_thread::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}