
# dovah

General library for interacting with DovahKit, and with loaded game data.

## Methods

<dl>
   <dt>dovah.count_forms_of_type(ft)</dt>
   <dd>
      Counts how many forms of type <var>ft</var> are loaded. If no game data has been loaded yet, or if <var>ft</var> is not a valid form type (from the <var>form_types</var> singleton), then this method returns zero.
   </dd>
   <dt>dovah.create_form(ft, options)</dt>
   <dd>
      <p>Attempts to create and return a new form of the specified type <var>ft</var>. Throws an error if no game data has been loaded yet, or if <var>ft</var> is not a valid form type (from the <var>form_types</var> singleton).</p>
      <p>The <var>options</var> argument is optional. If it is specified, and if it is a table, then the following fields are recognized:</p>
      <dl>
         <dt>editor_id</dt>
         <dd>If specified, the value is converted to a string and used as the new form's editor ID.</dd>
         <dt>grid_coordinates</dt>
         <dd>A table containing "x" and "y" fields, to indicate where in a worldspace's grid a new exterior cell should be created. If the values are not numbers or strings convertible to numbers, then they are treated as zero. If the values are not integers, then they are truncated. If <var>options.grid_coordinates</var> is not a table, then a warning is emitted.</dd>
         <dt>parent</dt>
         <dd>A parent form to use for the new form, e.g. a cell to put a new ObjectReference in, or a worldspace to put a new cell in. If this value is not a form, then a warning is emitted.</dd>
      </dl>
      <p>The following errors can be thrown if form creation fails:</p>
      <ul>
         <li>No game data is currently loaded.</li>
         <li>Invalid or unknown form type specified.</li>
         <li>The current load order does not have an active file, nor any room for a new active file.</li>
         <li>There are no more form IDs available in the active file.</li>
         <li>The specified parent form cannot have a child form of this type.</li>
         <li>Cannot create a new exterior cell at the specified grid coordinates in the specified worldspace, as there is already another cell there.</li>
         <li>Cannot create an ObjectReference without specifying a parent cell to place it in.</li>
         <li>DovahKit does not yet support loading or working with forms of the specified type. (This error should only be thrown while DovahKit is still in beta.)</li>
         <li>Cannot create a new form because the form ID that DovahKit wants to use is the target of one or more dangling references, and at least one is outbound from a form that DovahKit doesn't yet know how to load. (This error should only be thrown while DovahKit is still in beta.)</li>
      </ul>
   </dd>
   <dt>dovah.deep_stringify(value)</dt>
   <dd>
      Returns a string representation of the passed-in argument. If the argument is a table, then its full contents are recursively stringified. Tables are prefixed with a hash sign and a number the first time they are seen; if they appear again, then only the hash sign and number are printed. This means that the string representation is capable of representing structures in which a table is referred to more than once, including recursive structures.
   </dd>
   <dt>dovah.dump(value)</dt>
   <dd>
      Creates a string representation of the passed-in argument, through the same means as <code>dovah.deep_stringify(value)</code>, and prints that representation to the script message log.
   </dd>
   <dt>dovah.for_each_form_of_type(ft, functor)</dt>
   <dd>
      Loops over every form of type <var>ft</var>, calling the function <var>functor</var> for each form and passing the form as an argument. If <var>functor</var> returns the boolean <code>true</code>, then the loop stops early. If <var>ft</var> is not a valid form type (from the <var>form_types</var> singleton), does nothing; if <var>functor</var> is not a function, then throws an error.
   </dd>
   <dt>dovah.get_form_by_id(id)</dt>
   <dd>
      Returns the form with the numeric ID <var>id</var>, or nil if no such form exists. If <var>id</var> is not a number, throws an error; if <var>id</var> is not an integer, returns nil.
   </dd>
   <dt>dovah.log_message(format, ...)</dt>
   <dd>
      Prints a message to the script message log. If the first argument is not a string, it is coerced into a string. If additional arguments are present, then this function acts similarly to the standard <code>string.format</code> function.
   </dd>
   <dt>dovah.load_game_asset(options)</dt>
   <dd>
      <p>Attempts to load and return a game asset.</p>
      <p>If <var>options</var> is a string, then it is treated as a path that is relative to (and should not include) the Data directory. The function will return whatever asset is found, or nil if the asset does not exist.</p>
      <p>If <var>options</var> is a table or userdata, then it is checked for a "path" field and a "type" field. The "path" field is mandatory and must be a string or a value convertible to a string, containing a path relative to the Data directory. The "type" field is optional but, if specified, must be a string or a value convertible to a string. (An error is thrown if these requirements are not met.) This function will load the specified asset and return it if it is of the desired type; if the asset does not exist, is not of the desired type, or cannot be converted to the desired type, then this function returns nil. Known types are:</p>
      <ul>
         <li>audio</li>
         <li>binary</li>
         <li>image</li>
         <li>raster</li>
         <li>text</li>
         <li>bin</li>
         <li>bmp</li>
         <li>dds</li>
         <li>gif</li>
         <li>jpg</li>
         <li>jpeg</li>
         <li>png</li>
         <li>text</li>
      </ul>
      <p>An options object can be used when you want to retrieve an asset in a specific format (e.g. loading a PNG file as binary rather than as a raster, to work with bytes rather than pixels).</p>
   </dd>
   <dt>dovah.lookup_game_ini_setting(filename, category, setting)</dt>
   <dt>dovah.lookup_game_ini_setting({ filename = "", category = "", setting = "" })</dt>
   <dd>
      <p>Attempts to look up the value of the specified Skyrim INI setting. You must specify the INI file to load from, and the setting category and name. If the setting exists and is known to DovahKit (i.e. DovahKit is hardcoded to recognize it), then this function returns an object that you can use to manipulate the setting; otherwise, this function returns nil.</p>
      <p>The filename, category, and setting values that you pass in must be strings, or an error will be thrown. Allowed filenames are "Skyrim.ini" and "SkyrimPrefs.ini", case-insensitive; a disallowed filename will throw an error.</p>
   </dd>
   <dt>dovah.object_is(object, type)</dt>
   <dd>
      Checks if the passed-in <var>object</var> is of a given <var>type</var> or a subclass. Throws if <var>type</var> is not a string. The <var>type</var> strings should be the raw names used by DovahKit's classes and wrappers. Returns a boolean.
   </dd>
   <dt>dovah.type(object)</dt>
   <dd>
      Checks the type of the passed-in <var>object</var>, and returns it. "Type" here refers to DovahKit's classes and wrappers or, if the object isn't one of those, its Lua type; the return value will be a string.
   </dd>
</dl>

## Properties

<dl>
   <dt>package</dt>
   <dd>
      A library present if a script package is currently running; this value is nil otherwise. This property has its own documentation article.
   </dd>
   <dt>version</dt>
   <dd>
      A read-only userdata with "major", "minor", "patch", and "build" properties. This userdata is convertible to a string, and will produce a version string. The data is built from DovahKit's hardcoded version info (i.e. what you see if you right-click DovahKit.exe and view its properties and details in Windows).
   </dd>
</dl>