#include "./presence_cache.h"
#if _DEBUG
   #include <cassert>
#endif

namespace dovahkit::subsystems::form_info_cache {
   presence_cache::presence_cache() {
      #if _DEBUG
         this->_owning_thread = std::this_thread::get_id();
      #endif
   }

   void presence_cache::_verify_thread_affinity() const {
      #if _DEBUG
         assert(std::this_thread::get_id() == this->_owning_thread && "FIC data cache maps are only safe to access on their owning thread or for a *thread-safe* insertion specifically.");
      #endif
   }

   void presence_cache::threaded_insert(dovah::form_stub& stub) {
      const auto guard = std::unique_lock(this->_lock);

      auto it = std::find(this->_list.begin(), this->_list.end(), &stub);
      if (it != this->_list.end())
         return;
      this->_list.push_back(&stub);
   }

   bool presence_cache::insert(dovah::form_stub& stub) {
      _verify_thread_affinity();
      auto it = std::find(this->_list.begin(), this->_list.end(), &stub);
      if (it != this->_list.end())
         return false;
      this->_list.push_back(&stub);
      return true;
   }

   bool presence_cache::erase(const dovah::form_stub& stub) {
      _verify_thread_affinity();
      auto it = std::find(this->_list.begin(), this->_list.end(), &stub);
      if (it == this->_list.end()) {
         return false;
      }
      this->_list.erase(it);
      return true;
   }
}