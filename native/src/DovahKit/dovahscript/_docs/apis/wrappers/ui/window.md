
# ui.window

This is a subclass of `ui.widget`, and inherits all non-static members.

There is a limit on how many windows a script can create at once; as of this writing, that limit is 10.

A window's size is limited to no more than 90% of the size of the user's primary monitor.

This widget can contain other widgets.

## Instance methods

<dl>
   <dt>instance:hide()</dt>
   <dd>
      Closes the window.
   </dd>
   <dt>instance:show()</dt>
   <dd>
      Shows the window.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.has_help_button</dt>
   <dd>
      A boolean controlling whether a help button is made available in the title bar. If the button is visible, clicking it enters "What's This?" mode, where clicking on any control with non-blank <code>whats_this</code> text will display that text in an infobox.
   </dd>
   <dt>instance.has_size_handle</dt>
   <dd>
      A boolean controlling whether a resize handle is made available on the lower-right corner of the window.
   </dd>
   <dt>instance.height</dt>
   <dd>
      The height of the window in pixels, as a positive non-zero integer.
   </dd>
   <dt>instance.title</dt>
   <dd>
      A string containing the title bar text of the window.
   </dd>
   <dt>instance.width</dt>
   <dd>
      The width of the window in pixels, as a positive non-zero integer.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.window.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.window.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error. Newly-created windows are not shown by default.</p>
   </dd>
</dl>