#pragma once
#include <memory>
#include <vector>
#include <QString>
#include "./_base_node.h"
#include "./idle_node.h"
namespace dovah {
   class form_stub;
}

namespace IdleAnimationFormsModel_impl {
   class action_node : public node {
      public:
         action_node() : node(node_type::action) {}

         virtual size_t index_of_child(const node&) const override;
         virtual const node* nth_child(size_t) const override;
         virtual void update_cached_form_data() override;

      public:
         dovah::form_stub* stub = nullptr;
         struct {
            std::vector<std::unique_ptr<idle_node>> idles;
         } children;
         struct {
            QString editor_id;
         } cached;
   };
}