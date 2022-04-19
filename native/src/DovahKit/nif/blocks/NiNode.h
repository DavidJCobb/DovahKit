#pragma once
#include <type_traits>
#include "helpers/function_traits.h"
#include "NiAVObject.h"

namespace nifDK::block_types {
   class NiDynamicEffect;

   class NiNode : public NiAVObject {
      public:
         static constexpr const char* const type_name = "NiNode";
      public:
         std::vector<NiAVObject*>      children;
         std::vector<NiDynamicEffect*> effects;

         virtual void parse(file_reader&) override;

         size_t index_of_child(const NiAVObject*) const;

         template<typename T> requires std::is_base_of_v<NiAVObject, T>
         size_t count_descendants_of_type() const {
            size_t count = 0;
            for (auto* child : this->children) {
               auto* node = dynamic_cast<NiNode*>(child);
               if constexpr (std::is_base_of_v<NiNode, T>) {
                  //
                  // If T derives from NiNode, then we can skip the cast to T if the prior 
                  // cast to NiNode (which we need for recursion) failed.
                  //
                  if (!node)
                     continue;
                  if (auto* casted = dynamic_cast<T*>(child)) {
                     ++count;
                  }
               } else {
                  if (auto* casted = dynamic_cast<T*>(child)) {
                     ++count;
                  }
                  if (!node)
                     continue;
               }
               count += node->count_descendants_of_type<T>();
            }
            return count;
         }
         template<typename T> requires (!std::is_base_of_v<NiAVObject, T>)
         inline size_t count_descendants_of_type() const {
            //
            // The "children" vector can only store NiAVObjects; a NiNode by definition cannot have 
            // any descendants of a type that doesn't subclass NiAVObject.
            //
            return 0;
         }

         template<typename T> void for_self_and_subtree(T lambda) {
            (lambda)(this);
            for (auto* child : this->children) {
               auto* node = dynamic_cast<NiNode*>(child);
               if (!node)
                  continue;
               node->for_self_and_subtree(lambda);
            }
         }

      protected:
         template<typename T, bool outer> void _for_self_and_descendants(T lambda) {
            if constexpr (outer) {
               (lambda)(this);
            }
            for (auto* child : this->children) {
               (lambda)(child);
               if (auto* node = dynamic_cast<NiNode*>(child))
                  node->_for_self_and_descendants<T, false>(lambda);
            }
         }
      public:
         template<typename T> inline void for_self_and_descendants(T lambda) {
            _for_self_and_descendants<T, true>(lambda);
         }

         template<typename T> inline void for_non_node_descendants(T lambda) {
            for (auto* child : this->children) {
               if (auto* node = dynamic_cast<NiNode*>(child))
                  node->for_non_node_descendants(lambda);
               else
                  (lambda)(child);
            }
         }

         //
         // NiNode::walk_tree(state, node_func, child_func)
         // 
         //    Recurses over  this node and its  descendants,  running (node_func) on the node and all 
         //    descendant nodes, and (child_func) on all non-node descendants.  If (node_func) returns 
         //    a boolean,  then having it return false for a node  will skip all further operations on 
         //    that node and its subtree.
         // 
         //    Each  functor takes as its argument the target NetImmerse object and a copy of a  state 
         //    object.  The state object  is copied per-node, and is passed as  a non-const  reference 
         //    to (node_func),  and a const  reference to (child_func).  This means  that changes made 
         //    by a node to the state object are seen by the node's descendants, but not by the node's 
         //    siblings or ancestors. In essence,  it mimics argument passing within a recursive call, 
         //    and is useful for things like applying transforms to descendant nodes as one walks down 
         //    the node tree.  (If you want the functors to all live-update and share a single object, 
         //    then just use lambda captures.)
         //
         template<typename State, typename NodeFunctor, typename ChildFunctor, bool outer = true>
         requires requires(NiNode* node, NiAVObject* child, State& s, const State& const_s, NodeFunctor nf, ChildFunctor cf) {
            { nf(node, s) };
            { cf(child, const_s) };
         }
         void walk_tree(State s, NodeFunctor nf, ChildFunctor cf) {
            if constexpr (std::is_same_v<bool, cobb::function_traits<NodeFunctor>::return_type>) {
               bool filter = (nf)(this, s);
               if (!filter)
                  return;
            } else {
               (nf)(this, s);
            }
            for (auto* child : this->children) {
               if (!child) // can happen when loading a file with unidentified block types
                  continue;
               State inst = s;
               if (auto* node = dynamic_cast<NiNode*>(child))
                  node->walk_tree<State, NodeFunctor, ChildFunctor, false>(inst, nf, cf);
               else
                  (cf)(child, inst);
            }
         }
   };
}