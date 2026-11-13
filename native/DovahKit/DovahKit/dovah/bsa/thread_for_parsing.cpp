#include "./thread_for_parsing.h"
#include <stdexcept>
#include "lu/threads/current/set_name.h"
#include "lu/threads/max_name_length.h"
#include "./archive.h"
#include "./archive_collection.h"
#include "dovah/exceptions/bsa_parse_error.h"
#include "dovah/termination_handler.h"

namespace dovah::bsa {
   thread_for_parsing::thread_for_parsing(archive_collection& o) : owner(o) {
   }

   /*static*/ void thread_for_parsing::_exec(thread_for_parsing* instance) {
      termination_handler::update_this_thread();
      if constexpr (lu::threads::max_name_length > 16) {
         lu::threads::current::set_name("Dovah: Parse BSA");
      } else {
         lu::threads::current::set_name("dovah-parse-bsa");
      }

      auto& self = *instance;
      for (auto* archive : self.archives) {
         if (!archive)
            return;
         if (self.owner.is_load_aborted())
            break;
         try {
            archive->open();
         } catch ([[maybe_unused]] const std::system_error& e) {
            // TODO: log these
            continue;
         } catch ([[maybe_unused]] const exceptions::bsa_parse_error& e) {
            // TODO: log these
            continue;
         }
      }
      self.complete = true;
      self.owner._on_thread_complete({}, self);
   }

   void thread_for_parsing::add_archive(archive& archive) {
      this->archives.push_back(&archive);
   }
   
   void thread_for_parsing::start() {
      this->thread = std::thread(_exec, this);
   }
   void thread_for_parsing::wait_for() {
      this->thread.join();
   }

}