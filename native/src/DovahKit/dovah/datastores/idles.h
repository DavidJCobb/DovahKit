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
         struct _empty_t {};

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

         static bool _graph_node_sort_comparator(const graph_node*, const graph_node*);

         #pragma region Post-build updates
            void _push_idle_hierarchy_position_to_form(idle_node&, size_t i);
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
            struct {
               struct {
                  std::function<void(size_t)>      before;
                  std::function<void(graph_node&)> after;
               } on_inserted;
            } graphs;
            callbacks_by_type<action_node, action_parent_node> actions;
            callbacks_by_type<idle_node,   idle_parent_node>   idles;
            struct {
               std::function<void()> before;
               std::function<void()> after;
            } on_cleared;
            struct {
               std::function<void(form_stub&)> before;
               std::function<void(form_stub&)> after;
            } on_any_form_modified;
         } callbacks;
         std::vector<warning*> warnings; // owned
      protected:
         bool _is_building = false;

      public:
         #pragma region Graph node getters
            const graph_node* graph_by_idle(form_stub&) const noexcept;
            graph_node* graph_by_idle(form_stub&) noexcept;
            const graph_node* graph_by_path(std::string_view) const noexcept;
            graph_node* graph_by_path(std::string_view) noexcept;
            //
            graph_node* get_or_create_graph_by_path(std::string_view); // can fail and return nullptr for an empty path
         #pragma endregion
         #pragma region Loose action getters
            const action_node* loose_action(const form_stub&) const noexcept;
            action_node* loose_action(const form_stub&) noexcept;
            //
            action_node* get_or_create_loose_action(form_stub&);
         #pragma endregion

         const idle_node* idle_by_stub(const form_stub&) const noexcept;
         idle_node* idle_by_stub(const form_stub&) noexcept;

         #pragma region Hierarchy helpers
            const graph_node* graph_by_idle(const idle_node&) const noexcept;
            graph_node* graph_by_idle(idle_node&) noexcept;
         #pragma endregion

         #pragma region Handlers for events occurring outside the datastore
            void on_before_form_deleted(form_stub&); // only call if the form is actually deleted, not merely flagged as "deleted by override"

            void on_action_modified(form_stub&);

            void on_idle_created(form_stub&);
            void on_before_idle_deleted(form_stub&); // only call if the form is actually deleted, not merely flagged as "deleted by override"
            //
            // No "idle modified" callback is provided. If an idle has had its hierarchy 
            // data (parent and previous sibling) changed outside of this datastore -- 
            // outside of the relationships and invariants it maintains -- then you have 
            // no choice but to rebuild the datastore. The way erroneous hierarchy data 
            // gets handled by the engine is rather complicated, and trying to replicate 
            // that post-build is difficult... so we don't lol.
            //
         #pragma endregion

         // You should use only these to move a node. They will update loaded form data 
         // for the moved idle (i.e. parent/previous-sibling data) and may update data 
         // for adjacent nodes (i.e. previous-sibling relationships).
         bool place_idle_after(idle_node&, idle_node& desired_previous_sibling);
         bool append_idle_in(idle_node&, idle_parent_node& desired_parent);
   };
}