#pragma once
#include <memory>
#include <vector>
#include <QString>
#include "./_base_node.h"
#include "./idle_node.h"

namespace IdleAnimationFormsModel_impl {
   class loose_container_node : public node {
      public:
         loose_container_node() : node(node_type::loose_container) {}

      public:
         struct {
            std::vector<std::unique_ptr<idle_node>> idles;
         } children;
   };
}