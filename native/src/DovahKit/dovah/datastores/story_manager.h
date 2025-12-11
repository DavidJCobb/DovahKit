#pragma once
#include <functional>
#include <unordered_map>
#include <string_view>
#include <vector>
#include "../form_types.h"
namespace dovah {
   class file_load_order;
   class form_stub;
}
namespace dovah::datastores::impl::story_manager {
   class node;
   class branch_node;
   class leaf_node;
   //
   class warning;
}

namespace dovah::datastores {
   class story_manager {
      public:
         using node        = impl::story_manager::node;
         using branch_node = impl::story_manager::branch_node;
         using leaf_node   = impl::story_manager::leaf_node;

         using warning = impl::story_manager::warning;

      public:
         story_manager();
         ~story_manager();

      public:
         std::unordered_map<dovah::form_stub*, node*> nodes_by_stub; // nodes are owned
         branch_node* root = nullptr; // unowned (owned via `nodes_by_stub`); may be null
         struct {
            struct {
               std::function<void()> before;
               std::function<void()> after;
            } reset;
            struct {
               std::function<void(const node&, const branch_node& parent, size_t before_index)> before;
               std::function<void(node&)> after;
            } node_placed;
            struct {
               std::function<void(const node&)> before;
               std::function<void(uint32_t form_id)> after;
            } node_deleted;
            struct {
               std::function<void(form_stub&)> before;
               std::function<void(form_stub&)> after;
            } form_data_modified;
         } callbacks;
         struct {
            std::function<void(form_stub&)> delete_form;
         } handlers;
         std::vector<warning*> warnings; // owned

      protected:
         void _clear();
         static bool _is_form_type_relevant(form_type);

      public:
         void build(file_load_order&);
         void normalize_for_editing(); // call immediately after `build` if it suits your use case. does not emit callbacks.
         void reset();
         
         #pragma region Handlers for events occurring outside the datastore
            void on_before_form_fully_deleted(form_stub&); // only call if the form is actually deleted, not merely flagged as "deleted by override"

            void on_form_created(form_stub&);
         #pragma endregion

      public:
         const node* node_by_stub(const form_stub&) const noexcept;
         node* node_by_stub(const form_stub&) noexcept;

         bool is_node_movement_legal(const node& subject, const branch_node& dst_parent, const node* dst_previous) const;

      protected:

         // Prefer this over `node::_update_form_hierarchy_data`. This fires our 
         // form-modified callbacks and then calls into the `node` member function.
         void _update_form_data(node&);

         void _delete_single_node(node&);

      public:
         void delete_node(node&); // you must fill `handlers.delete_form` before calling this
         void move_node(node& subject, branch_node& dst_parent, node* dst_previous);
         void move_node_within_parent(node&, int by);
   };
}