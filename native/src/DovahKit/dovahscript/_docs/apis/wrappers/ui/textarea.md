
# ui.textarea

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a multi-line textbox.

## Events

### OnChanged

Fires when the textarea's value changes. Listeners receive the textarea's text, at the moment the event is fired, as an argument.

### OnCommitted

Fires when the user presses Ctrl + Enter while the textarea has keyboard focus. Listeners receive the textarea's text, at the moment the event is fired, as an argument.

### OnInputRejected

Fires when the user's attempts to type into the textarea are rejected, as a result of the value having reached its maximum length.

## Instance methods

<dl>
   <dt>instance:clear()</dt>
   <dd>
      Clears the textarea's value.
   </dd>
   <dt>instance:redo()</dt>
   <dd>
      Redoes an input that the user undid.
   </dd>
   <dt>instance:select_all()</dt>
   <dd>
      Selects the textarea's full contents.
   </dd>
   <dt>instance:undo()</dt>
   <dd>
      Undoes the user's last input.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.max_length</dt>
   <dd>
      An integer that controls the maximum length of the textarea's contents. As of this writing, this value is capped at 99999.
   </dd>
   <dt>instance.placeholder</dt>
   <dd>
      A string containing text displayed in the widget when the widget's value is empty.
   </dd>
   <dt>instance.read_only</dt>
   <dd>
      A boolean controlling whether the user is allowed to edit the textarea's value. A read-only textarea is not disabled; the user can select and copy its contents.
   </dd>
   <dt>instance.text</dt>
   <dd>
      A string containing the textarea's current value.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.textarea.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.textarea.new(text)</dt>
   <dd>
      <p>Creates and returns a new instance. You can optionally pass a string, to serve as the textarea's initial value.</p>
   </dd>
</dl>