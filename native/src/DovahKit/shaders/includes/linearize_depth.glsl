
#ifndef included_linearize_depth // include guard
#define included_linearize_depth

//
// The values in a depth buffer span the range [0, 1], but aren't linear: a fragment 
// at the exact center between the near and far planes will not necessarily produce 
// a depth buffer value of 0.5. This function linearizes depth buffer values.
//
// Changing the numerator of this fraction to (near * far) would give you an estimate 
// of the original Z-distance instead.
//
float linearize_depth(float depth, float near, float far) {
   return near / (far + depth * (near - far));
}

#endif // include guard