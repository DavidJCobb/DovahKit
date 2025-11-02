#pragma once
#include <functional>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace dovah {
   namespace loaded_forms {
      class IdleAnimation;
   }
   class file_load_order;
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class node;
   class action_node;
   class action_parent_node;
   class graph_node;
   class idle_parent_node;
   class idle_node;
   //
   class warning;
}

namespace dovah::datastores {
   class idles {
      public:
         using node = impl::idles::node;
         using action_node = impl::idles::action_node;
         using action_parent_node = impl::idles::action_parent_node;
         using graph_node = impl::idles::graph_node;
         using idle_node = impl::idles::idle_node;
         using idle_parent_node = impl::idles::idle_parent_node;

         using loaded_idle_type = dovah::loaded_forms::IdleAnimation;

         using warning = impl::idles::warning;

      protected:
         template<typename Node, typename ParentNode>
         struct callbacks_by_type {
            struct {
               std::function<void(const Node&)> before;
               std::function<void()> after;
            } on_deleted;
            struct {
               std::function<void(const ParentNode&, size_t)> before;
               std::function<void(Node&)> after;
            } on_inserted;
            struct {
               std::function<void(const ParentNode& from, size_t, const ParentNode& to, size_t)> before;
               std::function<void()> after;
            } on_moved;
            struct {
               std::function<void(const ParentNode&)> before;
               std::function<void(const ParentNode&, const std::vector<size_t>&)> after;
            } on_sorted;
         };

      public:
         idles();
         ~idles();

         void build(file_load_order&);
         void reset();

      protected:
         void _clear();

      protected:
         #pragma region Initial build
            void _place_parent_idle(idle_node&, const loaded_idle_type&);
            void _place_child_idle(idle_node&, const loaded_idle_type&);

            using seen_idle_set = std::set<dovah::form_stub*>;
            enum class sibling_problem {
               cyclical,
               ancestor,
               mismatched,
            };
            //
            bool _has_cyclical_parentage(seen_idle_set& seen_ancestors, const loaded_idle_type& idle);
            std::optional<sibling_problem> _check_siblings(const seen_idle_set& seen_ancestors, const loaded_idle_type& idle);

            void _post_placement_parentage_validation(idle_node&);
         #pragma endregion

      public:
         std::vector<graph_node*> graphs; // owned
         struct {
            action_parent_node* actions; // owned
            idle_parent_node*   idles;   // owned
         } loose;
         std::unordered_map<form_stub*, idle_node*> idles_by_stub; // unowned
         //
         struct {
            callbacks_by_type<action_node, action_parent_node> actions;
            callbacks_by_type<idle_node,   idle_parent_node>   idles;
            struct {
               std::function<void()> before;
               std::function<void()> after;
            } on_cleared;
         } callbacks;
         std::vector<warning*> warnings; // owned

      public:
         #pragma region Graph node getters
            const graph_node* graph_by_idle(dovah::form_stub&) const noexcept;
            graph_node* graph_by_idle(dovah::form_stub&) noexcept;
            const graph_node* graph_by_path(std::string_view) const noexcept;
            graph_node* graph_by_path(std::string_view) noexcept;
            //
            graph_node* get_or_create_graph_by_path(std::string_view); // can fail and return nullptr for an empty path
         #pragma endregion
         #pragma region Loose action getters
            const action_node* loose_action(dovah::form_stub&) const noexcept;
            action_node* loose_action(dovah::form_stub&) noexcept;
            //
            action_node* get_or_create_loose_action(dovah::form_stub&);
         #pragma endregion
   };
}