#pragma once
#include <type_traits>

namespace cobb::flat_tree_interface {
   template<typename Node>
   class tree {
      public:
         using node = Node;

      protected:
         template<bool Const>
         struct _recursive_iterator_base {
         };

      public:
         
   };
}
