#pragma once
#include <string_view>
#include <utility> // std::pair
#include "helpers/eight_cc.h"
namespace dovah::loaded_forms::components {
   class condition;
   class condition_list;
}
namespace dovahscript {
   class wrapper;
   namespace wrappers {
      struct condition;
   }
}
struct lua_State;

namespace dovahscript::wrapper_likes::native_lists {
   struct condition_list {
      condition_list() = delete;

      public:
         static constexpr const std::string_view class_name    = "native_list<condition>";
         static constexpr const std::string_view metatable_key = "collection<dovah.classes.land_texture.grasses>";

         using element_wrapper   = wrappers::condition;
         using wrapped_list_type = dovah::loaded_forms::components::condition_list;
         using wrapped_item_type = dovah::loaded_forms::components::condition;

         struct signatures {
            signatures() = delete;

            inline static constexpr cobb::eight_cc typical = "CtdnList";

            // Specific condition lists, on the handful of forms that have multiple:
            inline static constexpr cobb::eight_cc quest_dialogue = "CtdLQDia";
            inline static constexpr cobb::eight_cc quest_events   = "CtdLQEvt";
         };

      public:
         static void define_metatable(lua_State*);
         static int push(lua_State*, const wrapper& parent, cobb::eight_cc signature = signatures::typical);

      public: // helpers for the `condition` API's `unwrap` code
         static wrapped_list_type* unwrap_condition_list(wrapper&);
         static std::pair<wrapped_list_type*, size_t> unwrap_condition_list_and_index(wrapper&, size_t ctda_index);
         static std::pair<wrapped_list_type*, size_t> unwrap_condition_list_and_index(wrapper&);
   };
}