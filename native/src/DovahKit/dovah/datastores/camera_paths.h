#pragma once
#include <functional>
#include <unordered_map>
#include <set>
#include <string_view>
#include <vector>
#include "./camera_paths/node_parent.h"
#include "../form_types.h"
namespace dovah {
   namespace loaded_forms {
      class CameraPath;
   }
   class file_load_order;
   class form_stub;
}
namespace dovah::datastores::impl::camera_paths {
   class node;
   //
   class warning;
}

namespace dovah::datastores {
   class camera_paths {
      public:
         using node        = impl::camera_paths::node;
         using node_parent = impl::camera_paths::node_parent;

         using warning = impl::camera_paths::warning;

         static constexpr const form_type relevant_form_type = form_type::camera_path;
         using loaded_form_data = loaded_forms::CameraPath;

      public:
         camera_paths();
         ~camera_paths();

      public:
         std::unordered_map<dovah::form_stub*, node*> nodes_by_stub; // nodes are owned
         node_parent root;
         struct {
            struct {
               std::function<void()> before;
               std::function<void()> after;
            } reset;
            struct {
               std::function<void(const node&, const node_parent& parent, size_t before_index)> before;
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
            std::function<void(form_stub&)> delete_camera_path;
         } handlers;
         std::vector<warning*> warnings; // owned

      protected:
         void _clear();

      public:
         void build(file_load_order&);
         void reset();
         
         #pragma region Handlers for events occurring outside the datastore
            void on_before_form_fully_deleted(form_stub&); // only call if the form is actually deleted, not merely flagged as "deleted by override"

            void on_form_created(form_stub&);
         #pragma endregion

      protected:
         static bool _form_has_cyclical_parentage(std::set<form_stub*>&, loaded_form_data&);

         enum class sibling_problem {
            none,
            ancestor,
            cyclical,
            mismatched,
         };
         static sibling_problem _form_has_bad_siblinghood(const std::set<form_stub*>& seen_ancestors, loaded_form_data&);

         void _place_form(node&, loaded_form_data&);
         void _post_placement_parentage_validation(node&);

      public:
         const node* node_by_stub(const form_stub&) const noexcept;
         node* node_by_stub(const form_stub&) noexcept;

         bool is_node_movement_legal(const node& subject, const node_parent& dst_parent, const node* dst_previous) const;

      protected:

         // Prefer this over `node::_update_form_hierarchy_data`. This fires our 
         // form-modified callbacks and then calls into the `node` member function.
         void _update_form_data(node&);

         void _delete_single_node(node&);

      public:
         void delete_camera_path(node&); // you must fill `handlers.delete_idle` before calling this
         void move_camera_path(node& subject, node_parent& dst_parent, node* dst_previous);
         void move_camera_path_within_parent(node&, int by);
   };
}