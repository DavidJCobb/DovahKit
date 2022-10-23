#pragma once

namespace nifDK::block_types {
   class BSShaderProperty;
}
namespace vulkanDK {
   class rendered_mesh;
   class surface_renderer;
}

namespace vulkanDK::asset_loading {
   extern void reserve_mesh_textures_given_ni_shader(surface_renderer&, rendered_mesh&, nifDK::block_types::BSShaderProperty* shader);
}