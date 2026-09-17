#pragma once

namespace dovah::landscapes {
   // Vertices per quad side, including the vertex that is part of the shared edge (between 
   // two quads in the same landscape, or a quad in one landscape and the adjoining quad in 
   // the adjacent landscape).
   constexpr const size_t vertices_per_quad_side = 17;
}