#pragma once
namespace IdleAnimationFormsModel_impl {
   enum class node_type {
      graph,  // Havok behavior graph
      action,
      idle,

      loose_action_container,
      loose_idle_container,
   };
}
