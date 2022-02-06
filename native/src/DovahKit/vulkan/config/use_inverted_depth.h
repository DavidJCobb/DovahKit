#pragma once

namespace vulkanDK::config {
   //
   // Inverting the depth axis when rendering can improve precision within the depth buffer. NVIDIA 
   // has a good write-up on it: <https://developer.nvidia.com/content/depth-precision-visualized>
   //
   static constexpr bool use_inverted_depth = true;
}