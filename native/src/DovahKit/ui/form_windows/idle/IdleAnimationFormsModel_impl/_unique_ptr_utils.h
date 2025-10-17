#pragma once
#include <memory>
namespace IdleAnimationFormsModel_impl {
   class action_node;
   class graph_node;
   class idle_node;
   class loose_action_parent_node;
   class loose_idle_parent_node;
}

namespace IdleAnimationFormsModel_impl {
   struct node_deleter {
      void operator()(action_node*) const noexcept;
      void operator()(graph_node*) const noexcept;
      void operator()(idle_node*) const noexcept;
      void operator()(loose_action_parent_node*) const noexcept;
      void operator()(loose_idle_parent_node*) const noexcept;
   };

   template<typename T>
   using node_unique_ptr = std::unique_ptr<T, node_deleter>;

   template<typename T, typename... Args>
   [[nodiscard]] node_unique_ptr<T> make_unique(Args&&... args) {
      return node_unique_ptr<T>(new T(std::forward<Args>(args)...));
   }
}