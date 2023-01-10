
# vector3

A helper class representing a three-dimensional vector.

## Instance metamethods

<dl>
   <dt>__add (operator: +)</dt>
   <dd>If the operand is a table or userdata, then its "x", "y", and "z" properties are added to those of this vector, and the result is returned as a new vector3. Otherwise, an error is thrown.</dd>
   <dt>__div (operator: /)</dt>
   <dd>If the operand is a non-zero number or string convertible to a number, then this instance's "x", "y", and "z" properties are divided by this number, and the result is returned as a new vector3. Otherwise, an error is thrown.</dd>
   <dt>__mul (operator: *)</dt>
   <dd>If the operand is a number or a string convertible to a number, then this instance's "x", "y", and "z" properties are multiplied by this number, and the result is returned as a new vector3. Otherwise, an error is thrown.</dd>
   <dt>__sub (operator: -)</dt>
   <dd>If the operand is a table or userdata, then its "x", "y", and "z" properties are added to those of this vector, and the result is returned as a new vector3. Otherwise, an error is thrown.</dd>
   <dt>__tostring</dt>
   <dd>
      <p>Returns a string following the format <code>"(%f, %f, %f)"</code>, with the vector components listed alphabetically.
   </dd>
</dl>

## Instance methods

<dl>
   <dt>instance:add(operand)</dt>
   <dd>
      Behaves the same as the <code>__add</code> metamethod, but the result overwrites this vector. This vector is also returned, to allow method chaining.
   </dd>
   <dt>instance:copy()</dt>
   <dd>
      Creates and returns a new <code>vector3</code> instance that is identical to the original.
   </dd>
   <dt>instance:cross(operand)</dt>
   <dd>
      If the operand is a vector3 instance, then this method computes the cross product and returns it as a new vector3 instance.
   </dd>
   <dt>instance:div(operand)</dt>
   <dd>
      Behaves the same as the <code>__div</code> metamethod, but the result overwrites this vector. This vector is also returned, to allow method chaining.
   </dd>
   <dt>instance:dot(operand)</dt>
   <dd>
      If the operand is a vector3 instance, then this method computes and returns the dot product as a number. Otherwise, an error is thrown.
   </dd>
   <dt>instance:flatten(operand)</dt>
   <dd>
      If the operand is a vector3 instance, then this method projects <code>instance</code> onto <code>operand</code> and returns the length of the result, as a number. Otherwise, an error is thrown.
   </dd>
   <dt>instance:length(operand)</dt>
   <dd>
      Returns the length of this vector, as a number.
   </dd>
   <dt>instance:length_squared(operand)</dt>
   <dd>
      Returns the squared length of this vector, as a number. When doing comparisons in bulk (e.g. distance comparisons), it can be more efficient to compare the squared length of vectors to a squared threshold; taking the square root of a number is a relatively slow operation.
   </dd>
   <dt>instance:mul(operand)</dt>
   <dd>
      Behaves the same as the <code>__mul</code> metamethod, but the result overwrites this vector. This vector is also returned, to allow method chaining.
   </dd>
   <dt>instance:normalize()</dt>
   <dd>
      Normalizes this vector3, modifying it in-place. The instance is returned, to allow method chaining.
   </dd>
   <dt>instance:normalized()</dt>
   <dd>
      Normalizes this vector, and returns the result as a new vector3 instance.
   </dd>
   <dt>instance:project(operand)</dt>
   <dd>
      If the operand is a vector3 instance, then this method projects <code>instance</code> onto <code>operand</code>, overwrites <code>instance</code> with the result, and returns <code>instance</code> to allow method chaining.
   </dd>
   <dt>instance:projected(operand)</dt>
   <dd>
      If the operand is a vector3 instance, then this method projects <code>instance</code> onto <code>operand</code> and returns the result as a new vector3 instance. Otherwise, an error is thrown.
   </dd>
   <dt>instance:set_xyz(x, y, z)</dt>
   <dd>
      Modifies the <var>x</var>, <var>y</var>, and <var>z</var> properties on the vector3. This method exists because some APIs for form data may provide vector3s, and in some such cases, it may be more efficient to set the X, Y, and Z components all at once rather than individually.
   </dd>
   <dt>instance:sub(operand)</dt>
   <dd>
      Behaves the same as the <code>__sub</code> metamethod, but the result overwrites this vector. This vector is also returned, to allow method chaining.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.x</dt>
   <dd>
      The X-component. This value can be freely written to or modified, but if it is not a number or a string convertible to a number, then this class's methods will treat it as zero.
   </dd>
   <dt>instance.y</dt>
   <dd>
      The Y-component. This value can be freely written to or modified, but if it is not a number or a string convertible to a number, then this class's methods will treat it as zero.
   </dd>
</dl>

## Static methods

<dl>
   <dt>vector3.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>vector3.new(1, 2)</dt>
   <dt>vector3.new({ x = 2, y = 3 })</dt>
   <dd>
      <p>Creates and returns a vector3 filled with the supplied values.</p>
      <p>If the first argument is a number or a string convertible to a number, then the function accepts up to two arguments, treating them as X- and Y-components. If an argument is missing, or if it is neither a number nor a string convertible to a number, then zero is used in its place.</p>
      <p>If the first argument is a table or userdata <var>T</var>, then the function uses values <var>T.x</var> and <var>T.y</var>. If any value is missing, or is neither a number nor a string convertible to a number, then zero is used in its place.</p>
      <p>If no argument is provided, then the function returns a vector3 filled with all zeroes.</p>
   </dd>
</dl>