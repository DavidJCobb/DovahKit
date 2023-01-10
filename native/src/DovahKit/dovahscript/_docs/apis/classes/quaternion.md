
# Quaternion

A helper class for working with rotations encoded as quaternions.

## Definitions

A value is <dfn>quaternion-like</dfn> if it is a <code>quaternion</code> instance, or if it meets the following requirements:

* It must be a table or userdata.
* It must define fields named "x", "y", "z", and "w".
* Those fields must each store a number or a string convertible to a number.

## Instance metamethods

<dl>
   <dt>__add (operator: +)</dt>
   <dd>
      <p>Adds the operand to this quaternion, returning the result as a new quaternion instance. Throws an error if the operand is not quaternion-like.</p>
   </dd>
   <dt>__div (operator: /)</dt>
   <dd>
      <p>Divides this quaternion by the operand, returning the result as a new quaternion instance. Throws an error if the operand is not a number or a string convertible to a number.</p>
   </dd>
   <dt>__mul (operator: *)</dt>
   <dd>
      <p>If the operand is a number or a string convertible to a number, then this quaternion is multiplied by the operand, and the result is returned as a new quaternion instance. If the operand is quaternion-like, then the two quaternions are multiplied, and the result is returned as a new quaternion instance. Otherwise, an error is thrown.</p>
   </dd>
   <dt>__sub (operator: -)</dt>
   <dd>
      <p>Adds the operand to this quaternion, returning the result as a new quaternion instance. Throws an error if the operand is not quaternion-like.</p>
   </dd>
   <dt>__tostring</dt>
   <dd>
      <p>Returns a string following the format <code>"(%f, %f, %f, %f)"</code>, with the components listed in order WXYZ.</p>
   </dd>
</dl>

## Instance methods

<dl>
   <dt>instance:add(operand)</dt>
   <dd>
      Adds the operand, following the same rules as the <code>__add</code> metamethod but overwriting <code>instance</code> with the result. The instance is also returned, to allow method chaining.
   </dd>
   <dt>instance:conjugate()</dt>
   <dd>
      Creates and returns a new quaternion instance holding this quaternion's conjugate.
   </dd>
   <dt>instance:copy()</dt>
   <dd>
      Creates and returns a new <code>quaternion</code> instance that is identical to the original.
   </dd>
   <dt>instance:div(operand)</dt>
   <dd>
      Divides this quaternion by the operand, following the same rules as the <code>__div</code> metamethod but overwriting <code>instance</code> with the result. The instance is also returned, to allow method chaining.
   </dd>
   <dt>instance:inverse()</dt>
   <dd>
      Creates and returns a new quaternion instance holding this quaternion's inverse.
   </dd>
   <dt>instance:mul(operand)</dt>
   <dd>
      Multiplies this quaternion by the operand, following the same rules as the <code>__mul</code> metamethod but overwriting <code>instance</code> with the result. The instance is also returned, to allow method chaining.
   </dd>
   <dt>instance:norm()</dt>
   <dd>
      Computes and returns the quaternion's norm, as a number.
   </dd>
   <dt>instance:sub(operand)</dt>
   <dd>
      Subtracts the operand, following the same rules as the <code>__sub</code> metamethod but overwriting <code>instance</code> with the result. The instance is also returned, to allow method chaining.
   </dd>
   <dt>instance:to_euler()</dt>
   <dd>
      Creates and returns a new <code>euler</code> instance representing the same rotation, using lefthanded intrinsic XYZ Euler conventions.
   </dd>
   <dt>instance:to_matrix()</dt>
   <dd>
      Creates and returns a <code>matrix3x3</code> representing the same rotation.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.w</dt>
   <dd>
      The W-component. This value can be freely written to or modified, but if it is not a number or a string convertible to a number, then this class's methods will treat it as zero.
   </dd>
   <dt>instance.x</dt>
   <dd>
      The X-component. This value can be freely written to or modified, but if it is not a number or a string convertible to a number, then this class's methods will treat it as zero.
   </dd>
   <dt>instance.y</dt>
   <dd>
      The Y-component. This value can be freely written to or modified, but if it is not a number or a string convertible to a number, then this class's methods will treat it as zero.
   </dd>
   <dt>instance.z</dt>
   <dd>
      The Z-component. This value can be freely written to or modified, but if it is not a number or a string convertible to a number, then this class's methods will treat it as zero.
   </dd>
</dl>

## Static methods

<dl>
   <dt>quaternion.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>quaternion.new(1, 2, 3, 4)</dt>
   <dt>quaternion.new({ w = 1, x = 2, y = 3, z = 4 })</dt>
   <dd>
      <p>Creates and returns a quaternion filled with the supplied values.</p>
      <p>If the first argument is a number or a string convertible to a number, then the function accepts up to four arguments, treating them as W, X, Y, and Z. If an argument is missing, or if it is neither a number nor a string convertible to a number, then zero is used in its place.</p>
      <p>If the first argument is a table or userdata <var>T</var>, then the function uses values <var>T.w</var>, <var>T.x</var>, <var>T.y</var>, and <var>T.z</var>. If any value is missing, or is neither a number nor a string convertible to a number, then zero is used in its place.</p>
      <p>If no argument is provided, then the function returns a quaternion filled with all zeroes.</p>
   </dd>
</dl>