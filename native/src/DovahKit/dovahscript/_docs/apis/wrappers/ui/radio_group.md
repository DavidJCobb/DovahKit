
# ui.radio_group

An object used to group `radio_button` objects.

## Events

### OnSelectionChanged

Fires when the currently selected radio button within the group changes. Listeners receive the newly-selected radio button (if any is selected) as an argument.

## Instance methods

<dl>
   <dt>instance:on(event_name, listener_name, listener)</dt>
   <dd>
      Registers an event listener. Works the same way as the corresponding method on <code>widget</code>.
   </dd>
   <dt>instance:remove(button)</dt>
   <dd>
      Receives a <code>radio_button</code> as an argument, and removes the button from the group.
   </dd>
   <dt>instance:remove_event_listener(event_name, listener_name)</dt>
   <dd>
      Removes an event listener. Works the same way as the corresponding method on <code>widget</code>.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.selected_button</dt>
   <dd>
      The currently selected radio button in the group, or nil. This value can be set, but attempting to write in a button that isn't in the group will throw an error.
   </dd>
   <dt>instance.selected_id</dt>
   <dd>
      The radio group ID of the currently selected radio button in the group, or nil. This value can be set, but attempting to write in an ID that doesn't match any button in the group will throw an error.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.radio_group.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.radio_group.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>