#pragma once
#include <tuple>
#include "helpers/tuples/unpack_types_into.h"
#include "../../tool_response_tuple.h"

namespace dovahkit::subsystems::worldedit::tools::tandem {
   template<typename... Tools>
   class invoke_in_tandem {
      public:
         using all_tools = std::tuple<Tools...>;

      public:
         //static void _invoke_impl(const typename Tools::response*...);

         template<typename Self>
         static void invoke(const tool_response_tuple& responses) {
            Self::_invoke_impl(
               (responses.has_member<Tools>() ? &responses.get_member<Tools>() : nullptr)...
            );
         }
   };
}

namespace dovahkit::subsystems::worldedit::tools {
   template<typename T>
   concept is_tandem_invocation = requires {
      typename T::all_tools;
      requires std::is_base_of_v<
         cobb::tuples::unpack_types_into<typename T::all_tools, tandem::invoke_in_tandem>,
         T
      >;
   };
}