
# ui.textbox

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a single-line textbox.

## Events

### OnChanged

Fires when the textbox's value changes. Listeners receive the textbox's text, at the moment the event is fired, as an argument.

### AfterChanged

Fires when the user presses Enter while the textbox has keyboard focus, or when the textbox loses focus. Listeners receive the textbox's text, at the moment the event is fired, as an argument.

### OnInputRejected

Fires when the user's attempts to type into the textbox are rejected, as a result of the value having reached its maximum length or an entered character not matching the validation mask.

## Instance methods

<dl>
   <dt>instance:clear()</dt>
   <dd>
      Clears the textbox's value.
   </dd>
   <dt>instance:redo()</dt>
   <dd>
      Redoes an input that the user undid.
   </dd>
   <dt>instance:select_all()</dt>
   <dd>
      Selects the textbox's full contents.
   </dd>
   <dt>instance:undo()</dt>
   <dd>
      Undoes the user's last input.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.alignment</dt>
   <dd>
      A string controlling the alignment of text within the textbox.
   </dd>
   <dt>instance.max_length</dt>
   <dd>
      An integer that controls the maximum length of the textbox's contents.
   </dd>
   <dt>instance.placeholder</dt>
   <dd>
      A string containing text displayed in the widget when the widget's value is empty.
   </dd>
   <dt>instance.read_only</dt>
   <dd>
      A boolean controlling whether the user is allowed to edit the textbox's value. A read-only textbox is not disabled; the user can select and copy its contents.
   </dd>
   <dt>instance.text</dt>
   <dd>
      A string containing the textbox's current value.
   </dd>
   <dt>instance.validation_mask</dt>
   <dd>
      <p>A string used to limit what characters can be typed into the widget. The string represents a pattern, where certain characters are <dfn>mask characters</dfn> representing categories of symbols, and other characters are <dfn>meta characters</dfn> which alter the behavior of all following text. Any other symbols, including backslash-escaped symbols, are treated as separators that must be present.</p>
      <p>As an example, the mask <code>"000.000.000.000;_"</code> will restrict users to entering IPv4 addresses. The textbox, when blank, will display as <code>___.___.___.___</code>. When users type, non-numeric characters will be blocked, and numeric characters will be slotted into the blanks: if the user types three digits, the cursor will skip past the period after them; on the other hand, if the user types two digits and a dot, the cursor will skip to the next blank in the input mask.</p>
      <p>Mask characters:</p>
      <dl>
         <dt>A</dt>
         <dd>Symbol must be an alphabetical character.</dd>
         <dt>a</dt>
         <dd>Symbol may be an alphabetical character, but des not have to be.</dd>
         <dt>N</dt>
         <dd>Symbol must be an alphabetical character or numeric digit.</dd>
         <dt>n</dt>
         <dd>Symbol may be an alphabetical character or numeric digit, but des not have to be.</dd>
         <dt>X</dt>
         <dd>Symbol must not be whietspace.</dd>
         <dt>x</dt>
         <dd>Symbol may be non-whitespace, but des not have to be.</dd>
         <dt>9</dt>
         <dd>Symbol must be a numeric digit.</dd>
         <dt>0</dt>
         <dd>Symbol may be a numeric digit, but des not have to be.</dd>
         <dt>D</dt>
         <dd>Symbol must be a non-zero numeric digit.</dd>
         <dt>d</dt>
         <dd>Symbol may be a non-zero numeric digit, but des not have to be.</dd>
         <dt>#</dt>
         <dd>Symbol may be a numeric digit, plus sign, or minus sign, but des not have to be.</dd>
         <dt>H</dt>
         <dd>Symbol must be a hexadecimal digit i.e. 0 through 9 or A through F, case-insensitive.</dd>
         <dt>h</dt>
         <dd>Symbol may be a hexadecimal digit, but does not have to be.</dd>
         <dt>B</dt>
         <dd>Symbol must be a binary digit i.e. 0 or 1.</dd>
         <dt>bh</dt>
         <dd>Symbol may be a binary digit, but does not have to be.</dd>
      </dl>
      <p>Meta characters:</p>
      <dl>
         <dt>&gt;</dt>
         <dd>All following alphabetic characters are forced to uppercase.</dd>
         <dt>&lt;</dt>
         <dd>All following alphabetic characters are forced to lowercase.</dd>
         <dt>!</dt>
         <dd>Case conversion stops.</dd>
         <dt>;<var>c</var></dt>
         <dd>Can be placed at the end of the input mask, to indicate the end of the input mask. Replace <var>c</var> with whatever the blank character should be (i.e. the character that the mask slots are pre-filled with). If this doesn't appear, the default blank character is a space.</dd>
         <dt>[</dt>
         <dt>]</dt>
         <dt>{</dt>
         <dt>}</dt>
         <dd>Reserved for future use by the Qt UI library.</dd>
         <dt>\</dt>
         <dd>Use to escape characters, so they can be used as separators.</dd>
      </dl>
   </dd>
   <dt>value_visibility</dt>
   <dd>
      <p>A string controlling how the textbox's value is displayed. The following values are allowed:</p>
      <dl>
         <dt>normal</dt>
         <dd>The textbox has no special behavior for displaying its value.</dd>
         <dt>invisible</dt>
         <dd>The textbox doesn't display its value; it always appears blank.</dd>
         <dt>password</dt>
         <dd>The textbox displays asterisks, bullets, or other placeholder symbols instead of the ones you've actually typed.</dd>
         <dt>password char-by-char</dt>
         <dd>The same as <code>password</code>, except that the last-typed symbol is displayed verbatim.</dd>
      </dl>
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.textbox.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.textbox.new(text)</dt>
   <dd>
      <p>Creates and returns a new instance. You can optionally pass a string, to serve as the textbox's initial value.</p>
   </dd>
</dl>