#pragma once

namespace IdleAnimationFormsModel_impl {
   enum class node_type {
      graph,  // Havok behavior graph
      action,
      idle,
      
      // Container node for Idles whose parents are either an 
      // Action, or non-existent.
      loose_container,
   };
   
   class node {
      protected:
         constexpr node(node_type t) : type(t) {}
      public:
         const node_type type;
         node* parent = nullptr;
   };
}
