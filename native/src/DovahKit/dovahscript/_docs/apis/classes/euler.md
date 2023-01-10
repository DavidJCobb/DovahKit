
# euler

A helper class for working with rotations encoded as Euler angles.

## Instance methods

<dl>
   <dt>instance:copy()</dt>
   <dd>
      Creates and returns a new <code>euler</code> instance that is identical to the original.
   </dd>
   <dt>instance:to_matrix()</dt>
   <dd>
      Creates and returns a <code>matrix3x3</code> representing the same rotation, assuming lefthanded intrinsic XYZ as the Euler convention.
   </dd>
   <dt>instance:to_quaternion()</dt>
   <dd>
      Creates and returns a <code>quaternion</code> representing the same rotation, assuming lefthanded intrinsic XYZ as the Euler convention.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.x</dt>
   <dd>
      Returns the X-axis degrees of rotation. This value can be read or written to, but a written value must be a number or a string convertible to a number.
   </dd>
   <dt>instance.y</dt>
   <dd>
      Returns the Y-axis degrees of rotation. This value can be read or written to, but a written value must be a number or a string convertible to a number.
   </dd>
   <dt>instance.z</dt>
   <dd>
      Returns the Z-axis degrees of rotation. This value can be read or written to, but a written value must be a number or a string convertible to a number.
   </dd>
</dl>

## Static methods

<dl>
   <dt>euler.from_degrees(1, 2, 3)</dt>
   <dt>euler.from_degrees({ 1, 2, 3 })</dt>
   <dt>euler.from_degrees({ x = 1, y = 2, z = 3 })</dt>
   <dt>euler.from_degrees({ 1, 2, z = 3 })</dt>
   <dd>
      <p>Creates and returns a new instance, using the supplied values as angles of rotation measured in degrees.</p>
      <p>If the first argument is a number, then the function will accept up to three arguments, assuming they are numbers and using them as X, Y, and Z values. Missing arguments, and arguments that are neither numbers nor strings convertible to numbers, are treated as zero.</p>
      <p>If the first argument is a table <var>T</var>, then angles will be extracted from it. The function will check for named fields (<var>T.x</var>, <var>T.y</var>, and <var>T.z</var>), falling back to indexed fields (<var>T[1]</var>, <var>T[2]</var>, <var>T[3]</var>) if a named field is absent or if the field is neither a number nor a string convertible to a number. If no indexed field is found, a warning is emitted.</p>
   </dd>
   <dt>euler.from_radians(1, 2, 3)</dt>
   <dt>euler.from_radians({ 1, 2, 3 })</dt>
   <dt>euler.from_radians({ x = 1, y = 2, z = 3 })</dt>
   <dt>euler.from_radians({ 1, 2, z = 3 })</dt>
   <dd>
      <p>Creates and returns a new instance, using the supplied values as angles of rotation measured in radians.</p>
      <p>If the first argument is a number, then the function will accept up to three arguments, assuming they are numbers and using them as X, Y, and Z values. Missing arguments, and arguments that are neither numbers nor strings convertible to numbers, are treated as zero.</p>
      <p>If the first argument is a table <var>T</var>, then angles will be extracted from it. The function will check for named fields (<var>T.x</var>, <var>T.y</var>, and <var>T.z</var>), falling back to indexed fields (<var>T[1]</var>, <var>T[2]</var>, <var>T[3]</var>) if a named field is absent or if the field is neither a number nor a string convertible to a number. If no indexed field is found, a warning is emitted.</p>
   </dd>
   <dt>euler.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>euler.new()</dt>
   <dd>
      Creates and returns a new instance, with all angles set to zero. Passing any arguments to this function is an error.
   </dd>
</dl>
