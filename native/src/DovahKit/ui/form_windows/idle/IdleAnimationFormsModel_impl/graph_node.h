#pragma once
#include <memory>
#include <vector>
#include <QString>
#include "./_base_node.h"
#include "./action_node.h"
#include "./loose_container_node.h"

namespace IdleAnimationFormsModel_impl {
   class graph_node : public node {
      public:
         graph_node();

         virtual size_t index_of_child(const node&) const override;
         virtual const node* nth_child(size_t) const override;
         virtual void update_cached_form_data() override;

      public:
         QString path; // behavior graph file path
         struct {
            std::vector<std::unique_ptr<action_node>> actions;
            std::unique_ptr<loose_container_node> loose;
         } children;
   };
}