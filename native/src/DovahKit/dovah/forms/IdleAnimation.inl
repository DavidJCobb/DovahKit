#pragma once
#include "./IdleAnimation.h"

namespace dovah::loaded_forms {
   constexpr const std::string& IdleAnimation::get_behavior_graph_path(bool verbatim) const noexcept {
      auto& pair = this->_hierarchy.behavior_graph;
      return verbatim ? pair.verbatim : pair.corrected;
   }

   constexpr form_stub* IdleAnimation::get_hierarchy_parent() const noexcept {
      return this->_hierarchy.parent.get_form_stub();
   }
   constexpr form_stub* IdleAnimation::get_hierarchy_previous_sibling() const noexcept {
      return this->_hierarchy.previous_sibling.get_form_stub();
   }
}