
# ui.groupbox

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a groupbox: a container for other widgets, which draws a rounded-rectangular outline around the bunch. The widget can optionally have a label embedded into its outline, and this label can be made checkable. If the label is checkable, then the widget's contents are automatically disabled whenever the label is unchecked. Disabling the widget itself will disable the checkbox in the label.

This widget can contain other widgets.

## Events

### OnToggled

Fired when the checkbox in the label, if any, has its state changed. Event listeners receive one argument: a boolean indicating whether the label is currently checked.

## Instance properties

<dl>
   <dt>alignment</dt>
   <dd>
      A string containing the alignment of the groupbox's label.
   </dd>
   <dt>checkable</dt>
   <dd>
      A boolean controlling whether the groupbox's label is checkable.
   </dd>
   <dt>checked</dt>
   <dd>
      A boolean indicating whether the groupbox's label is checked.
   </dd>
   <dt>text</dt>
   <dd>
      A string containing the text of the groupbox's label.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.groupbox.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.groupbox.new(text)</dt>
   <dd>
      <p>Creates and returns a new groupbox instance. You can optionally pass a single argument: a string, to be used as the groupbox's label text.</p>
   </dd>
</dl>