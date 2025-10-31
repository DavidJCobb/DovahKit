#pragma once
#include <concepts>
#include <type_traits>
#include <vector>
#include "helpers/type_traits/is_std_vector.h"
#include "../files/file_load_order.h"
#include "../form_stub.h"
#include "../forms/ActorAction.h"
#include "../forms/IdleAnimation.h"

namespace dovah::utils {
   namespace impl::idle_tree_builder {
      template<typename GraphNode, typename ActionNode, typename IdleNode>
      concept node_requirements = requires(
         GraphNode& graph,
         ActionNode& action,
         IdleNode& idle,
         const loaded_forms::ActorAction& action_form,
         const loaded_forms::IdleAnimation& idle_form
      ) {
         { graph.loose } -> std::same_as<std::vector<ActionNode*>>; // owned
         { action.stub } -> std::same_as<dovah::form_stub&>;
         { action.parent };
         { idle.stub } -> std::same_as<dovah::form_stub&>;
         { idle.parent };
         { ActionNode(action_form) };
         { IdleNode(idle_form) };

         requires []() -> bool {
            using action_parent_node = std::remove_pointer_t<decltype(ActionNode::parent)>;
            return requires (
               action_parent_node& parent,
               const action_parent_node & const_parent,
               ActionNode& action,
               const dovah::form_stub& stub
            ) {
               requires std::is_base_of_v<action_parent_node, GraphNode>;
               requires std::is_default_constructible_v<action_parent_node>;
               { parent.children }  -> std::same_as<std::vector<ActionNode*>>; // owned
               { parent.append_child(action) };
               { parent.get_action(stub) } -> std::same_as<ActionNode*>;
               { const_parent.get_action(stub) } -> std::same_as<const ActionNode*>;
               { parent.get_or_create_action(stub) } -> std::same_as<ActionNode*>;
               { parent.sort_children() };
               { parent.sort_descendants() }; // should not sort direct children of "loose" container nodes
            };
         }();
         requires []() -> bool {
            using idle_parent_node       = std::remove_pointer_t<decltype(IdleNode::parent)>;
            using loose_idle_parent_node = std::remove_pointer_t<decltype(GraphNode::loose)>;
            return requires (
               idle_parent_node& parent,
               const idle_parent_node& const_parent,
               IdleNode& idle,
               loose_idle_parent_node& loose_parent,
               size_t index
            ) {
               requires std::is_base_of_v<idle_parent_node, loose_idle_parent_node>;
               requires std::is_base_of_v<idle_parent_node, ActionNode>;
               requires std::is_base_of_v<idle_parent_node, IdleNode>;
               requires std::is_default_constructible_v<loose_idle_parent_node>;
               { parent.children } -> std::same_as<std::vector<IdleNode*>>; // owned
               { parent.append_child(idle) }; // if `idle` already has a parent node, this should use `take_child` to remove it from there
               { const_parent.index_of_child(idle) } -> std::same_as<size_t>;
               { parent.destroy_child(index) };
               { parent.take_child(index) } -> std::same_as<IdleNode*>;
               { parent.sort_children() };
               { parent.sort_descendants() };
               { loose_parent.owner };
            };
         }();
      };
   };

   template<
      typename GraphNode,
      typename ActionNode,
      typename IdleNode
   >
      requires impl::idle_tree_builder::node_requirements<GraphNode, ActionNode, IdleNode>
   class idle_tree_builder {
      public:
         using graph_node  = GraphNode;
         using action_node = ActionNode;
         using idle_node   = IdleNode;

         using action_parent_node = std::remove_pointer_t<decltype(action_node::parent)>;
         using idle_parent_node   = std::remove_pointer_t<decltype(idle_node::parent)>;

         using loose_idle_parent_node = std::remove_pointer_t<decltype(graph_node::loose)>;

         using loaded_idle_type = loaded_forms::IdleAnimation;
         using idle_flag = loaded_idle_type::flag;

      public:
         idle_tree_builder();
         ~idle_tree_builder();

         std::vector<graph_node*> graphs; // owned
         struct {
            action_parent_node*     actions = nullptr; // owned
            loose_idle_parent_node* idles   = nullptr; // owned
         } loose;
         std::unordered_map<dovah::form_stub*, idle_node*> idles_by_stub; // unowned
         //
         std::vector<dovah::form_stub*> cyclical_idles; // unowned

      protected:
         idle_node& _get_or_create_idle_node(loaded_idle_type&, idle_parent_node& in_parent);
         idle_node& _get_or_create_idle_node(loaded_idle_type&);
         void _build_parent_idle(loaded_idle_type&);
         void _build_child_idle(loaded_idle_type&);

         using seen_idle_set = std::set<dovah::form_stub*>;

         bool _has_cyclical_parentage(seen_idle_set& seen, const loaded_idle_type& idle);

         enum class sibling_problem {
            cyclical,
            mismatched,
         };
         std::optional<sibling_problem> _check_siblings(seen_idle_set& seen, const loaded_idle_type& idle);

      public:
         void build(file_load_order& lo);

         #pragma region Graph node getters
            const graph_node* graph_by_idle(dovah::form_stub&) const noexcept;
            graph_node* graph_by_idle(dovah::form_stub&) noexcept;
            const graph_node* graph_by_path(std::string_view) const noexcept;
            graph_node* graph_by_path(std::string_view) noexcept;
            //
            graph_node* get_or_create_graph_by_path(std::string_view);
         #pragma endregion

         #pragma region Loose action getters
            const action_node* loose_action(dovah::form_stub&) const noexcept;
            action_node* loose_action(dovah::form_stub&) noexcept;
            //
            action_node* get_or_create_loose_action(dovah::form_stub&);
         #pragma endregion
   };
}

#include "idle_tree_builder.inl"