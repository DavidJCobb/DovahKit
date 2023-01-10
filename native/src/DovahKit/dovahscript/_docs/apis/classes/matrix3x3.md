
# matrix3x3

A helper class for working with 3x3 matrices, including rotation matrices.

## Instance metamethods

<dl>
   <dt>__mul (operator: *)</dt>
   <dd>
      <p>If the other operand is a number, then this operator returns a new matrix3x3 instance, whose values are equal to those in this matrix but multiplied by the operand.</p>
      <p>If the other operand is a matrix3x3 instance, then this operator performs matrix multiplication, returning the result as a new matrix3x3 instance.</p>
      <p>If the other operand is a vector3 instance, then this operator performs matrix-by-column multiplication, returning the result as a new vector3 instance.</p>
      <p>All other operand types throw an error.</p>
   </dd>
</dl>

## Instance methods

<dl>
   <dt>instance:copy()</dt>
   <dd>
      Creates and returns a new <code>matrix3x3</code> instance that is identical to the original.
   </dd>
   <dt>instance:determinant()</dt>
   <dd>
      Computes and returns the matrix's determinant.
   </dd>
   <dt>instance:mul(operand)</dt>
   <dd>
      <p>Performs the same operations as the <code>__mul</code> metamethod, but if the return type would be a matrix3x3 instance, then this method modifies and returns <code>instance</code>, instead of creating and returning a new matrix.</p>
   </dd>
   <dt>instance:set_row(arg, n)</dt>
   <dd>
      If <var>arg</var> is a table or userdata, this method replaces the <var>n</var>-th row of the matrix with <var>arg[1]</var>, <var>arg[2]</var>, and <var>arg[3]</var>. If any of those three values is not a number or a string convertible to a number, then zero is used instead. If <var>arg</var> is the wrong type, then an error is thrown.
   </dd>
   <dt>instance:to_euler()</dt>
   <dd>
      Creates and returns a new <code>euler</code> instance representing the same rotation, using lefthanded intrinsic XYZ Euler conventions.
   </dd>
   <dt>instance:to_quaternion()</dt>
   <dd>
      Creates and returns a <code>quaternion</code> representing the same rotation.
   </dd>
   <dt>instance:trace()</dt>
   <dd>
      Computes and returns the matrix's trace.
   </dd>
   <dt>instance:transpose()</dt>
   <dd>
      Creates and returns a transposed copy of the matrix.
   </dd>
   <dt>instance:transpose_in_place()</dt>
   <dd>
      Modifies this matrix by transposing it, and then returns it.
   </dd>
</dl>

## Static methods

<dl>
   <dt>matrix3x3.construct_from_extrinsic_zyx(x, y, z)</dt>
   <dd>
      Creates and returns a matrix3x3 instance representating a rotation by the provided Euler angles, in radians, assuming lefthanded extrinsic ZYX (equivalent to lefthanded intrinsic XYZ) as the Euler convention. Provided arguments must be numbers or strings convertible to numbers; throws an error otherwise.
   </dd>
   <dt>matrix3x3.construct_from_x(rads, righthanded)</dt>
   <dd>
      Creates and returns a matrix3x3 instance representing a rotation by <var>rads</var> radians about the X-axis. If <var>righthanded</var> is a truthy value, then the rotation is righthanded; otherwise, it's lefthanded. The provided angle in radians must be a number or a string convertible to a number, or an error will be thrown.
   </dd>
   <dt>matrix3x3.construct_from_y(rads, righthanded)</dt>
   <dd>
      Creates and returns a matrix3x3 instance representing a rotation by <var>rads</var> radians about the Y-axis. If <var>righthanded</var> is a truthy value, then the rotation is righthanded; otherwise, it's lefthanded. The provided angle in radians must be a number or a string convertible to a number, or an error will be thrown.
   </dd>
   <dt>matrix3x3.construct_from_z(rads, righthanded)</dt>
   <dd>
      Creates and returns a matrix3x3 instance representing a rotation by <var>rads</var> radians about the Z-axis. If <var>righthanded</var> is a truthy value, then the rotation is righthanded; otherwise, it's lefthanded. The provided angle in radians must be a number or a string convertible to a number, or an error will be thrown.
   </dd>
   <dt>matrix3x3.identity()</dt>
   <dd>
      Creates and returns a matrix3x3 instance containing an identity matrix &mdash; that is, one where the diagonals from top-left to bottom-right are 1, and all other values are 0.
   </dd>
   <dt>matrix3x3.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>matrix3x3.new(1, 2, 3, 4, 5, 6, 7, 8, 9)</dt>
   <dt>matrix3x3.new({ 1, 2, 3, 4, 5, 6, 7, 8, 9 })</dt>
   <dt>matrix3x3.new({ { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } })</dt>
   <dd>
      <p>Creates and returns a matrix3x3 filled with the supplied values.</p>
      <p>If the first argument is a number or a string convertible to a number, then the function accepts up to nine arguments, filling the matrix from left to right, top to bottom. If an argument is missing, or if it is neither a number nor a string convertible to a number, then zero is used in its place.</p>
      <p>If the first argument is a table <var>T</var>, and <var>T[1]</var> is also a table, then the first argument is assumed to be a two-dimensional 3x3 array of values (i.e. an array of three rows). The values from this array are copied into the matrix; if any value is neither a number nor a string convertible to a number, then zero is used in its place. If a row is missing entirely, then zeroes are used for the values it would have contained.</p>
      <p>If the first argument is a table <var>T</var>, and <var>T[1]</var> is not a table, then the first argument is assumed to be a flat array of nine values. The values from this array are copied into the matrix, filling the matrix from left to right, top to bottom. If any value is neither a number nor a string convertible to a number, then zero is used in its place.</p>
   </dd>
</dl>