#pragma once
#include <memory>
#include <QString>
#include "./action_parent_node.h"

namespace IdleAnimationFormsModel_impl {
   class graph_node final : public action_parent_node {
      public:
         graph_node();
         ~graph_node();

         virtual void update_cached_form_data() override;

         void insert_loose_idle(node_unique_ptr<idle_node>&&);

      public:
         QString path; // behavior graph file path
         struct {
            node_unique_ptr<loose_idle_parent_node> idles;
         } loose;
   };
}