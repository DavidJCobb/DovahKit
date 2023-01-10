
# ui.text

This is a subclass of `ui.widget`, and inherits all non-static members.

## Instance methods

<dl>
   <dt>instance:clear()</dt>
   <dd>
      Clears the label's text.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.alignment</dt>
   <dd>
      A string controlling the alignment of text within the widget.
   </dd>
   <dt>instance.allow_selection</dt>
   <dd>
      A boolean controlling whether the user can select text in the widget.
   </dd>
   <dt>instance.font</dt>
   <dd>
      The <code>font</code> used to display the widget's text.
   </dd>
   <dt>instance.text</dt>
   <dd>
      A string containing the text to be displayed in the widget.
   </dd>
   <dt>instance.word_wrap</dt>
   <dd>
      A boolean controlling whether text in the widget word wraps.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.text.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.text.new(text)</dt>
   <dd>
      <p>Creates and returns a new instance. You can optionally pass a string argument, to set the text to be displayed in the widget.</p>
   </dd>
</dl>