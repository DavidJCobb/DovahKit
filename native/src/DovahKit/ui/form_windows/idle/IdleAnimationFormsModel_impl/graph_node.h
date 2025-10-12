#pragma once
#include <memory>
#include <vector>
#include <QString>
#include "./_base_node.h"
#include "./action_node.h"
#include "./idle_node.h"

namespace IdleAnimationFormsModel_impl {
   class graph_node : public node {
      public:
         graph_node() : node(node_type::graph) {}

      public:
         QString path; // behavior graph file path
         struct {
            std::vector<std::unique_ptr<action_node>> actions;
            std::vector<std::unique_ptr<idle_node>> loose_idles;
         } children;
   };
}