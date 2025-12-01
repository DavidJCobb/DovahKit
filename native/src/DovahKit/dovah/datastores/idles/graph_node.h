#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class action_node;
   class loose_idle_list_node;
   namespace passkeys {
      class initial_build;
      class post_build_edit;
   }
}

namespace dovah::datastores::impl::idles {
   class graph_node : public node {
      public:
         graph_node(datastore_type& d, std::string_view path);
         ~graph_node();

      public:
         const std::string path;
         std::vector<cobb::const_forwarding_ptr<action_node>> actions; // owned
         loose_idle_list_node* loose = nullptr; // owned

      public:
         static bool action_sort_comparator(const action_node*, const action_node*);

      public:
         const action_node* get_action(form_stub&) const noexcept;
         action_node* get_action(form_stub&) noexcept;

         size_t index_of_action(const action_node&) const noexcept;
         size_t index_of_action(const form_stub&) const noexcept;

         // What index would a node be placed at, were it to be inserted?
         size_t prospective_index_of(const action_node&) const noexcept;
         size_t prospective_index_of(const form_stub&) const noexcept;

         action_node* get_or_create_action(form_stub&);

         void re_sort_all_actions(passkeys::initial_build);
         void re_sort_action(passkeys::post_build_edit, size_t);

         // Check if this graph's path is equal to a provided path, accounting for 
         // letter casing and redundant directory separators.
         bool path_equals(std::string_view) const noexcept;
   };
}