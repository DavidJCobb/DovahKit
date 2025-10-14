#pragma once
#include <memory>
#include <stdexcept>
#include "../concepts/can_hold_nodes_of_type.h"
#include "./get_child_list.h"

namespace IdleAnimationFormsModel_impl::utils {
   template<
      concepts::can_hold_action_nodes ParentNode,
      typename BeforeFunctor, // argument: index we'll be inserting the child at
      typename AfterFunctor,
      typename ChildNode
   >
      requires (
         concepts::can_hold_nodes_of_type<ParentNode, ChildNode> &&
         requires(BeforeFunctor&& before, AfterFunctor&& after, size_t size) {
            { before(size) };
            { after() };
         }
      )
   std::unique_ptr<ChildNode> take_child(ParentNode& parent, ChildNode& child, BeforeFunctor&& before, AfterFunctor&& after) {
      if (child.parent != &parent) [[unlikely]]
         return;

      auto& list = get_child_list<ChildNode>(parent);
      auto  it   = std::find(list.begin(), list.end(), &child);
      assert(it != list.end());
      before(std::distance(list.begin(), it));
      auto  child_ptr = std::move(*it);
      child_ptr->parent = nullptr;
      list.erase(it);
      after();
      return child_ptr;
   }

   template<
      typename ChildNode,
      concepts::can_hold_action_nodes ParentNode,
      typename BeforeFunctor, // argument: index we'll be inserting the child at
      typename AfterFunctor
   >
      requires (
         concepts::can_hold_nodes_of_type<ParentNode, ChildNode> &&
         requires(BeforeFunctor&& before, AfterFunctor&& after, size_t size) {
            { before(size) };
            { after() };
         }
      )
   std::unique_ptr<ChildNode> take_child(ParentNode& parent, size_t index, BeforeFunctor&& before, AfterFunctor&& after) {
      auto& list = get_child_list<ChildNode>(parent);
      if (index >= list.size())
         throw std::out_of_range("Invalid `take_child` call.");
      before(index);
      auto child_ptr = std::move(*it);
      child_ptr->parent = nullptr;
      list.erase(list.begin() + index);
      after();
      return child_ptr;
   }
}