#pragma once
#include <QString>
#include "./idle_parent_node.h"
namespace dovah {
   class form_stub;
}
namespace IdleAnimationFormsModel_impl {
   class action_parent_node;
}

namespace IdleAnimationFormsModel_impl {
   class action_node final : public idle_parent_node {
      public:
         action_node() : idle_parent_node(node_type::action) {}

         virtual void update_cached_form_data() override;

      public:
         action_parent_node* parent = nullptr;
         dovah::form_stub*   stub   = nullptr;
         struct {
            QString editor_id;
         } cached;
   };
}