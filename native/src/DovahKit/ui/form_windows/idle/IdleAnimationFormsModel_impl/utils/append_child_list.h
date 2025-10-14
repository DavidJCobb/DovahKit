#pragma once
#include <memory>
#include <vector>
#include "../concepts/can_hold_nodes_of_type.h"
#include "./get_child_list.h"

namespace IdleAnimationFormsModel_impl::utils {
   template<
      concepts::can_hold_action_nodes ParentNode,
      typename BeforeInsertFunctor, // arguments: parent's child count prior to insertion; count to insert
      typename AfterInsertFunctor,
      typename ChildNode
   >
      requires (
         concepts::can_hold_nodes_of_type<ParentNode, ChildNode> &&
         requires(BeforeInsertFunctor&& before, AfterInsertFunctor&& after, size_t size) {
            { before(size, size) };
            { after() };
         }
      )
   void append_child_list(
      ParentNode& parent,
      std::vector<std::unique_ptr<ChildNode>>&& child_ptr_list,
      BeforeInsertFunctor&& before,
      AfterInsertFunctor&& after
   ) {
      auto&  list       = get_child_list<ChildNode>(parent);
      size_t size_prior = list.size();
      list.reserve(size_prior + child_ptr_list.size());
      before(size_prior, child_ptr_list.size());
      for (auto& child_ptr : child_ptr_list) {
         child_ptr->parent = &parent;
         list.push_back(std::move(child_ptr));
      }
      after();
   }
}