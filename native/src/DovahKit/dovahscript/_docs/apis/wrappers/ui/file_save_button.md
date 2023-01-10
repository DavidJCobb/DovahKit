
# ui.file_save_button

This is a subclass of `ui.widget`, and inherits all non-static members. This is a button that the user can click on, to open a Save As dialog and save a resource generated or made available by scripts.

## Instance properties

<dl>
   <dt>data</dt>
   <dd>
      The data that the user will be prompted to save. This can be any of the following types: a Lua string; <code>binary_view</code>; <code>dds_resource</code>; <code>raster</code>; <code>unknown_resource</code>.
   </dd>
   <dt>filename</dt>
   <dd>
      <p>A string containing the default filename that will be pre-filled-in in the Save As dialog. The user can change the filename at their discretion.</p>
      <p>When setting this value, if the filename contains any path separators (forward slashes or backslashes), then all content up to and including the last such separator will be removed from the string. If the file extension (here defined as anything after the last period, assuming the period comes after all path separators) is identified as indicating an executable file (e.g. EXE), then it will be forcibly changed to "bin"; if the file extension is identified as indicating any file format that Windows is capable of running as a shell script or similar, then it will be forcibly changed to "txt".</p>
   </dd>
   <dt>label</dt>
   <dd>
      <p>A string containing the text label shown on the button.</p>
      <p>If this value is an empty string, then the button will display the label <code>"Save file..."</code>. Otherwise, the button will display this value prefixed with the text <code>"Save file: "</code>.</p>
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.file_save_button.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.file_save_button.new()</dt>
   <dd>
      <p>Creates and returns a new widget. Passing any arguments to this function will throw an error.</p>
   </dd>
</dl>