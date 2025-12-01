#pragma once
#include <functional>
#include <unordered_map>
#include <set>
#include <string_view>
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
   class graph_node;
   class idle_node;
   class loose_idle_list_node;
   //
   class warning;
}

namespace dovah::datastores {
   class idles {
      public:
         using node                 = impl::idles::node;
         using action_node          = impl::idles::action_node;
         using graph_node           = impl::idles::graph_node;
         using idle_node            = impl::idles::idle_node;
         using loose_idle_list_node = impl::idles::loose_idle_list_node;

         using warning = impl::idles::warning;

         using loaded_idle_data = loaded_forms::IdleAnimation;

      public:
         idles();
         ~idles();

      public:
         std::unordered_map<dovah::form_stub*, idle_node*> idles_by_stub; // idles owned
         std::vector<graph_node*> graphs; // owned
         loose_idle_list_node*    loose = nullptr; // owned
         struct {
            struct {
               std::function<void()> before;
               std::function<void()> after;
            } reset;
            struct {
               std::function<void(const graph_node&, size_t)> before;
               std::function<void(const graph_node&)> after;
            } graph_inserted;
            struct {
               std::function<void(const action_node&)> before;
               std::function<void()> after;
            } action_deleted;
            struct {
               std::function<void(const graph_node&, const action_node&, size_t)> before;
               std::function<void(action_node&)> after;
            } action_inserted;
            struct {
               std::function<void(const idle_node&, const node& parent, size_t moved_to_index)> before;
               std::function<void(idle_node&)> after;
            } idle_moved;
            struct {
               std::function<void(const idle_node&)> before;
               std::function<void(uint32_t form_id)> after;
            } idle_deleted;
            std::function<void(idle_node&, action_node&)> idle_becoming_multiply_present_in;
            std::function<void(idle_node&, action_node&)> idle_no_longer_multiply_present_in;
            struct {
               std::function<void(form_stub&)> before;
               std::function<void(form_stub&)> after;
            } form_data_modified;
         } callbacks;
         struct {
            std::function<void(form_stub&)> delete_idle;
         } handlers;
         std::vector<warning*> warnings; // owned

      protected:
         void _clear();

      public:
         void build(file_load_order&);
         void reset();
         
         #pragma region Handlers for events occurring outside the datastore
            void on_before_form_fully_deleted(form_stub&); // only call if the form is actually deleted, not merely flagged as "deleted by override"

            void on_action_modified(form_stub&);

            void on_idle_created(form_stub&);
            void on_idle_editor_id_potentially_changed(form_stub&);
            //
            // No "idle modified" callback is provided. If an idle has had its hierarchy 
            // data (parent and previous sibling) changed outside of this datastore -- 
            // outside of the relationships and invariants it maintains -- then you have 
            // no choice but to rebuild the datastore. The way erroneous hierarchy data 
            // gets handled by the engine is rather complicated, and trying to replicate 
            // that post-build is difficult... so we don't lol.
            //
         #pragma endregion

      protected:
         graph_node* _get_or_create_graph(std::string_view);
         action_node* _get_or_create_action(std::string_view graph, form_stub* action);
         void _place_action_root(idle_node&, loaded_idle_data&);
         void _place_forced_loose_idle(idle_node&, loaded_idle_data&);
         static bool _idle_has_cyclical_parentage(std::set<form_stub*>&, loaded_idle_data&);

         enum class sibling_problem {
            none,
            ancestor,
            cyclical,
            mismatched,
         };
         static sibling_problem _idle_has_bad_siblinghood(const std::set<form_stub*>& seen_ancestors, loaded_idle_data&);

         void _place_child_idle(idle_node&, loaded_idle_data&);
         void _post_placement_parentage_validation(idle_node&);

      public:
         const graph_node* graph_by_path(std::string_view) const noexcept;
         graph_node* graph_by_path(std::string_view) noexcept;
         const graph_node* graph_by_idle(idle_node&) const noexcept;
         graph_node* graph_by_idle(idle_node&) noexcept;
         const graph_node* graph_by_idle(form_stub&) const noexcept;
         graph_node* graph_by_idle(form_stub&) noexcept;

         // Post-build.
         graph_node* get_or_create_graph(std::string_view);

         const idle_node* idle_by_stub(const form_stub&) const;
         idle_node* idle_by_stub(const form_stub&);

         bool is_idle_movement_legal(const idle_node& subject, const node& dst_parent, const idle_node* dst_previous) const;
         bool is_idle_movement_a_really_bad_idea(const idle_node& subject, const node& dst_parent, const idle_node* dst_previous) const;

         bool is_idle_deletion_legal(const idle_node&) const;
         bool is_idle_deletion_a_really_bad_idea(const idle_node&) const;

      protected:
         void _on_runner_up_became_root(action_node&);
         void _destroy_non_canonical_active_root_candidacies(idle_node&);
         idle_node* _take_idle_from_canonical_parent(node& take_from, idle_node&); // returns previous sibling, if any
         void _update_canonical_parent_action_after_root_taken(action_node& taken_from, idle_node& taken_idle);

         // Prefer this over `idle_node::_update_form_hierarchy_data`. This fires our 
         // form-modified callbacks and then calls into the `idle_node` member function.
         void _update_form_data(idle_node&);

         void _delete_single_idle(idle_node&);

      public:
         void delete_idle(idle_node&); // you must fill `handlers.delete_idle` before calling this
         void move_idle(idle_node& subject, node& dst_parent, idle_node* dst_previous);
         void move_idle_within_parent(idle_node&, int by);
   };
}