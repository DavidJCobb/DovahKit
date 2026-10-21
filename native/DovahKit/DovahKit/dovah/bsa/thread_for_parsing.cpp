#include "./thread_for_parsing.h"
#include <stdexcept>
#include "./archive.h"
#include "./archive_collection.h"
#include "dovah/exceptions/bsa_parse_error.h"
#include "dovah/termination_handler.h"

namespace dovah::bsa {
   thread_for_parsing::thread_for_parsing(archive_collection& o) : owner(o) {
   }

   /*static*/ void thread_for_parsing::_exec(thread_for_parsing* instance) {
      termination_handler::update_this_thread();

      auto& self = *instance;
      for (auto* archive : self.archives) {
         if (!archive)
            return;
         if (self.owner.is_load_aborted())
            break;
         try {
            archive->open();
         } catch (const std::system_error& e) {
            // TODO: log these
            continue;
         } catch (const exceptions::bsa_parse_error& e) {
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