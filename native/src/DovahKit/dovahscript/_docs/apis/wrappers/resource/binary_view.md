
# binary_view

An object that can be used to read and manipulate binary data, similar to JavaScript's DataView API.

As of this writing, scripts are not allowed to enlarge these objects above 1GB. Assets larger than 1GB can still be loaded, and can be resized to any smaller size even if that size is above 1GB; however, a binary view that is not already larger than 1GB cannot be made larger than 1GB.

## Instance methods

All "read" and "write" methods accept an offset and an endianness value.

* These methods throw an error if the offset is unspecified, not an integer, negative, or would result in reading or writing past the end of the binary view.
* These methods throw an error if the endianness is specified but is not the string "little" or the string "big". If unspecified, little-endian is the default.

All "append" methods accept an endianness value operating according to the same rules.

<dl>
   <dt>instance:append_bool(options)</dt>
   <dt>instance:append_bool(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by one byte. If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not a boolean.
   </dd>
   <dt>instance:append_double(options)</dt>
   <dt>instance:append_double(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by eight bytes. If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not a number.
   </dd>
   <dt>instance:append_float(options)</dt>
   <dt>instance:append_float(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by four bytes. If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not a number.
   </dd>
   <dt>instance:append_int8(options)</dt>
   <dt>instance:append_int8(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by 8 bits (one byte). If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not an integer in the range [-128, 127].
   </dd>
   <dt>instance:append_int16(options)</dt>
   <dt>instance:append_int16(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by 16 bits (two bytes). If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not an integer in the range [-32768, 32767].
   </dd>
   <dt>instance:append_int32(options)</dt>
   <dt>instance:append_int32(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by 32 bits (four bytes). If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not an integer in the range [-2147483648, 2147483647].
   </dd>
   <dt>instance:append_uint8(options)</dt>
   <dt>instance:append_uint8(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by 8 bits (one byte). If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not an integer in the range [0, 255].
   </dd>
   <dt>instance:append_uint16(options)</dt>
   <dt>instance:append_uint16(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by 16 bits (two bytes). If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not an integer in the range [0, 65535].
   </dd>
   <dt>instance:append_uint32(options)</dt>
   <dt>instance:append_uint32(value, endianness)</dt>
   <dd>
      Appends the value to the binary view, enlarging it by 32 bits (four bytes). If an options argument is specified, it must be a table or userdata with fields "value" and, optionally, "endian". Throws an error if the value is not an integer in the range [0, 4294967295].
   </dd>
   <dt>instance:get_bool(options)</dt>
   <dt>instance:get_bool(offset, endianness)</dt>
   <dd>
      Reads a single byte at the specified offset, and returns it as a boolean (non-zero values are true; zero is false). If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_double(options)</dt>
   <dt>instance:get_double(offset, endianness)</dt>
   <dd>
      Reads eight bytes at the specified offset, and returns a double-precision floating-point number. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_float(options)</dt>
   <dt>instance:get_float(offset, endianness)</dt>
   <dd>
      Reads four bytes at the specified offset, and returns a single-precision floating-point number. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_int8(options)</dt>
   <dt>instance:get_int8(offset, endianness)</dt>
   <dd>
      Reads 8 bits (one byte) at the specified offset, and returns a signed integer. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_int16(options)</dt>
   <dt>instance:get_int16(offset, endianness)</dt>
   <dd>
      Reads 16 bits (two bytes) at the specified offset, and returns a signed integer. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_int32(options)</dt>
   <dt>instance:get_int32(offset, endianness)</dt>
   <dd>
      Reads 32 bits (four bytes) at the specified offset, and returns a signed integer. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_uint8(options)</dt>
   <dt>instance:get_uint8(offset, endianness)</dt>
   <dd>
      Reads 8 bits (one byte) at the specified offset, and returns an unsigned integer. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_uint16(options)</dt>
   <dt>instance:get_uint16(offset, endianness)</dt>
   <dd>
      Reads 16 bits (two bytes) at the specified offset, and returns an unsigned integer. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:get_uint32(options)</dt>
   <dt>instance:get_uint32(offset, endianness)</dt>
   <dd>
      Reads 32 bits (four bytes) at the specified offset, and returns an unsigned integer. If an options argument is specified, it must be a table or userdata with fields "offset" and, optionally, "endian".
   </dd>
   <dt>instance:reserve(n)</dt>
   <dd>
      Reserves additional memory for the binary view, so that it can hold <var>n</var> bytes. If you expect to append large amounts of data to a binary view, then reserving that space ahead of time can speed up successive "append" calls.
   </dd>
   <dt>instance:resize(n)</dt>
   <dd>
      Resizes the binary view to contain <var>n</var> bytes. If the binary view is larger than that size, then its contents are truncated. If the binary view is smaller than that size, then the new space is filled with zeroes.
   </dd>
   <dt>instance:set_bool(options)</dt>
   <dt>instance:set_bool(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting one byte. If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not a boolean.
   </dd>
   <dt>instance:set_double(options)</dt>
   <dt>instance:set_double(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting eight bytes. If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not a number.
   </dd>
   <dt>instance:set_float(options)</dt>
   <dt>instance:set_float(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting four bytes. If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not a number.
   </dd>
   <dt>instance:set_int8(options)</dt>
   <dt>instance:set_int8(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting 8 bits (one byte). If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not an integer in the range [-128, 127].
   </dd>
   <dt>instance:set_int16(options)</dt>
   <dt>instance:set_int16(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting 16 bits (two bytes). If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not an integer in the range [-32768, 32767].
   </dd>
   <dt>instance:set_int32(options)</dt>
   <dt>instance:set_int32(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting 32 bits (four bytes). If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not an integer in the range [-2147483648, 2147483647].
   </dd>
   <dt>instance:set_uint8(options)</dt>
   <dt>instance:set_uint8(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting 8 bits (one byte). If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not an integer in the range [0, 255].
   </dd>
   <dt>instance:set_uint16(options)</dt>
   <dt>instance:set_uint16(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting 16 bits (two bytes). If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not an integer in the range [0, 65535].
   </dd>
   <dt>instance:set_uint32(options)</dt>
   <dt>instance:set_uint32(value, offset, endianness)</dt>
   <dd>
      Writes the specified value to the specified offset, overwriting 32 bits (four bytes). If an options argument is specified, it must be a table or userdata with fields "value",  "offset", and, optionally, "endian". Throws an error if the value is not an integer in the range [0, 4294967295].
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.size</dt>
   <dd>
      The instance's size in bytes, as a number. Setting this value is the same as calling <code>instance:resize(n)</code>.
   </dd>
</dl>

## Static methods

<dl>
   <dt>binary_view.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>binary_view.new(size)</dt>
   <dd>
      Creates and returns a new binary view. If <var>size</var> is specified, it must be an integer, and will be used as the initial size in bytes of the created view; otherwise, the initial size is zero. The created view will contain all zeroes.
   </dd>
</dl>