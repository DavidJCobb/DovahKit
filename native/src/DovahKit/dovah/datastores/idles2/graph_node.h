#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles2 {
   class action_node;
   class loose_idle_list_node;
}

namespace dovah::datastores::impl::idles2 {
   class graph_node : public node {
      public:
         graph_node(datastore_type& d, std::string_view path);
         ~graph_node();

      public:
         const std::string path;
         std::vector<cobb::const_forwarding_ptr<action_node>> actions; // owned
         loose_idle_list_node* loose = nullptr; // owned

      public:
         const action_node* get_action(form_stub&) const noexcept;
         action_node* get_action(form_stub&) noexcept;

         action_node* get_or_create_action(form_stub&);

         // Check if this graph's path is equal to a provided path, accounting for 
         // letter casing and redundant directory separators.
         bool path_equals(std::string_view) const noexcept;
   };
}