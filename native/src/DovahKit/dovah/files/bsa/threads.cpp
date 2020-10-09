#include "threads.h"
#include "bsa_archive.h"
#include "bsa_load_order.h"

namespace dovah {
   bsa_threaded_reader::bsa_threaded_reader(bsa_load_order& o) : owner(o) {
   }

   /*static*/ void bsa_threaded_reader::_exec(bsa_threaded_reader& self) {
      for (auto* archive : self.archives) {
         if (!archive)
            return;
         if (self.owner.is_archive_load_aborted())
            break;
         archive->open();
      }
      self.complete = true;
      self.owner.on_thread_complete(self);
   }

   void bsa_threaded_reader::add_archive(bsa_archive* archive) {
      this->archives.push_back(archive);
   }
   
   void bsa_threaded_reader::start() {
      this->thread = std::thread(_exec, *this);
   }
   void bsa_threaded_reader::wait_for() {
      this->thread.join();
   }

}