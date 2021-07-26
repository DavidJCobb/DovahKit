#pragma once
#include "../../../helpers/singleton.h"
#include "../../../lua.h"

class ObservableStandardItemModelObserver;
namespace dovah {
   class form_stub;
}
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::core::subsystems {
   class userdata : cobb::singleton {
      public:
         inline static userdata& get() {
            static userdata instance;
            return instance;
         }

      public:
         // Call when setting up a new Lua state.
         void initialize(lua_State*);

         void destroy(wrapper&);
         void destroy_all(dovah::form_stub&);
         void destroy_all(ObservableStandardItemModelObserver&);

         bool wrapper_exists_for(void*);

         void on_wrapper_destroyed(wrapper&);

         //
         // Check if Lua already has an identical copy of the passed-in wrapper;  if so, push that copy 
         // onto the Lua stack. Otherwise, copy the passed-in wrapper into Lua and push it onto the Lua 
         // stack.
         //
         int push(lua_State*, const wrapper&, const char* metatable_name);

         void remove_from_sequential_collection(wrapper& to_remove);

         //
         // Zombifies the wrappers for all elements in a collection, and for all of their descendants, 
         // without zombifying the wrapper for the collection itself. You would want to call this from 
         // an API that clears a collection's contents.
         //
         void clear_entire_collection(wrapper& collection_wrapper);
   };
}