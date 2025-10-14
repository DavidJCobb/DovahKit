#pragma once
#include <cassert>
#include <memory>
#include "../concepts/can_hold_nodes_of_type.h"
#include "./get_child_list.h"
#include "./take_child.h"

namespace IdleAnimationFormsModel_impl::utils {

   // Overload which takes the child by reference. The child must have a parent node (so that we 
   // know where to find its unique_ptr, and thus can move from it).
   template<
      concepts::can_hold_action_nodes ParentNode,
      typename BeforeInsertFunctor, // argument: index we'll be inserting the child at
      typename AfterInsertFunctor,
      typename ChildNode
   >
      requires (
         concepts::can_hold_nodes_of_type<ParentNode, ChildNode> &&
         requires(BeforeInsertFunctor&& before, AfterInsertFunctor&& after, size_t insert_at) {
            { before(insert_at) };
            { after() };
         }
      )
   void insert_sorted_child(ParentNode& parent, ChildNode& child, BeforeInsertFunctor&& before, AfterInsertFunctor&& after) {
      node* prior_parent = child->parent;
      assert(prior_parent != nullptr);
      if (prior_parent == &parent)
         return;
      auto child_ptr = take_child(*prior_parent, child);

      auto&  list      = get_child_list<ChildNode>(parent);
      size_t insert_at = list.size(); // TODO: alphabetical sort
      list.reserve(list.size() + 1);

      before(insert_at);
      child.parent = &parent;
      list.insert(list.begin() + insert_at, std::move(child_ptr));
      after();
   }

   // Overload which takes a unique_ptr to the child. The child does not need to have a parent 
   // node.
   template<
      concepts::can_hold_action_nodes ParentNode,
      typename BeforeInsertFunctor, // argument: index we'll be inserting the child at
      typename AfterInsertFunctor,
      typename ChildNode
   >
      requires (
         concepts::can_hold_nodes_of_type<ParentNode, ChildNode> &&
         requires(BeforeInsertFunctor&& before, AfterInsertFunctor&& after, size_t insert_at) {
            { before(insert_at) };
            { after() };
         }
      )
   void insert_sorted_child(ParentNode& parent, std::unique_ptr<ChildNode>&& child_ptr_arg, BeforeInsertFunctor&& before, AfterInsertFunctor&& after) {
      assert(child_ptr_arg != nullptr);
      node* prior_parent = child_ptr_arg->parent;
      if (prior_parent == &parent)
         return;

      auto&  list      = get_child_list<ChildNode>(parent);
      size_t insert_at = list.size(); // TODO: alphabetical sort
      list.reserve(list.size() + 1);

      before(insert_at);
      auto& child = *child_ptr_arg;
      if (prior_parent) {
         list.insert(list.begin() + insert_at, take_child(*prior_parent, child));
      } else {
         list.insert(list.begin() + insert_at, std::move(child_ptr_arg));
      }
      // `child_ptr_arg` is no longer valid.
      child.parent = &parent;
      after();
   }
}