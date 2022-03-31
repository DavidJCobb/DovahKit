
#ifndef included_linearize_depth // include guard
#define included_linearize_depth

//
// The values in a depth buffer span the range [0, 1], but aren't linear: a fragment 
// at the exact center between the near and far planes will not necessarily produce 
// a depth buffer value of 0.5. This function linearizes depth buffer values.
//
float linearize_depth(float depth, float near, float far) {
   return (near * far) / (far + depth * (near - far));
}

#endif // include guard