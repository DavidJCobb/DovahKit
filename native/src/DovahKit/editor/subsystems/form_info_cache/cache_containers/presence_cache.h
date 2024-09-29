#pragma once
#include <mutex>
#include <vector>
#if _DEBUG
   #include <thread>
#endif

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::form_info_cache {
   //
   // A flat list of form stubs that allows the following accessors:
   // 
   //  - Thread-safely insert a form stub into the list. (This is the only 
   //    thread-safe operation.)
   // 
   //  - Test for the presence of a form stub in the list.
   // 
   //  - Remove a form stub from the list.
   //
   class presence_cache {
      protected:
         std::mutex _lock;
         std::vector<dovah::form_stub*> _list;

         #if _DEBUG
            std::thread::id _owning_thread;
         #endif
         void _verify_thread_affinity() const;

      public:
         presence_cache();
         //
         // std::mutex isn't copyable, so default construct/assign behavior e.g. (*this = {}) has 
         // to be defined manually:
         //
         presence_cache(const presence_cache& src) : presence_cache() {
            this->_list = src._list;
         }
         presence_cache(presence_cache&& src) noexcept : presence_cache() {
            this->_list = std::move(src._list);
         }
         presence_cache& operator=(const presence_cache& src) {
            this->_list = src._list;
            return *this;
         }
         presence_cache& operator=(presence_cache&& src) noexcept {
            this->_list = std::move(src._list);
            return *this;
         }

         // This is the only thread-safe accessor.
         void threaded_insert(dovah::form_stub&);

         // Returns `true` if the stub wasn't already in the list; `false` otherwise.
         bool insert(dovah::form_stub&);

         // Returns `true` if the stub was in the list; `false` otherwise.
         bool erase(const dovah::form_stub&);

         constexpr bool contains(const dovah::form_stub& stub) const {
            auto it = std::find(this->_list.begin(), this->_list.end(), &stub);
            return (it != this->_list.end());
         }
   };
}