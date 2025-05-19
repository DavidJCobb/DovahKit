#pragma once
#include <condition_variable>
#include <mutex>
#include "helpers/passkey.h"

namespace dovahscript::core {
   namespace subsystems {
      class coordinator;
   }
}

namespace dovahscript::impl {
   //
   // The std::atomic<T> class offers some basic synchronization functions by way of the 
   // std::atomic<T>::wait(...), which allows you to wait until the atomic variable's value 
   // is no longer equal to the passed-in argument. (This isn't automatic: some other code, 
   // generally the code which is actually changing the value, must also call the "notify" 
   // member functions.) Unfortunately, however, although there's a function to wait until 
   // the variable *stops* being equal to some value, there's no function to wait until the 
   // variable *becomes* equal to some value.
   // 
   // If that ever gets added, we can use std::atomic<int> instead of this class.
   //
   class borrow_counter {
      public:
         using value_type      = int;
         using difference_type = value_type;

      protected:
         std::condition_variable cvar;
         std::mutex lock;
         //
         value_type counter = 0;

      public:
         template<typename pre_functor_t, typename post_functor_t> requires (std::is_invocable_v<pre_functor_t> && std::is_invocable_v<post_functor_t>)
         void wait_until_zero(pre_functor_t pre, post_functor_t post) {
            std::unique_lock guard(this->lock);
            if (this->counter) {
               pre();
               this->cvar.wait(guard, [this]() { return this->counter == 0; });
               post();
            }
         }

         value_type operator++() noexcept;
         value_type operator--() noexcept;
   };
}