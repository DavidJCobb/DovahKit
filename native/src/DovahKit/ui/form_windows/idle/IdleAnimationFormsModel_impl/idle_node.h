#pragma once
#include <QString>
#include "./idle_parent_node.h"
namespace dovah {
   class form_stub;
}

namespace IdleAnimationFormsModel_impl {
   class idle_node final : public idle_parent_node {
      public:
         idle_node() : idle_parent_node(node_type::idle) {}

         virtual void update_cached_form_data() override;

      public:
         idle_parent_node* parent = nullptr;
         dovah::form_stub* stub   = nullptr;
         struct {
            QString editor_id;
         } cached;
   };
}