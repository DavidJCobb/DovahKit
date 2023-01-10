
# ui.tabbo

This is a subclass of `ui.widget`, and inherits all non-static members.

## Events

### OnSelectionChanged

Fired when the currently selected tab changes. Listeners receive the tab's body widget as an argument.

## Instance methods

<dl>
   <dt>instance:add_tab(name)</dt>
   <dd>
      Adds a new tab to the end of the tabbox. If <var>name</var> is a string, then its value is the displayed name of the tab. Returns the tab's body widget.
   </dd>
   <dt>instance:insert_tab(position, name)</dt>
   <dd>
      Adds a new tab at <var>position</var>, which must be a positive non-zero integer. If <var>name</var> is a string, then its value is the displayed name of the tab. Returns the tab's body widget.
   </dd>
   <dt>instance:remove_tab(tab)</dt>
   <dd>
      Removes the specified tab. The argument can be a tab index (as a positive non-zero integer) or a <code>tabbox_tab</code> object.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>allow_reordering</dt>
   <dd>
      A boolean controlling whether the user is allowed to reorder tabs by dragging them.
   </dd>
   <dt>selected_index</dt>
   <dd>
      An integer indicating the currently selected tab.
   </dd>
   <dt>selected_tab</dt>
   <dd>
      The currently selected tab, as a <code>tabbox_tab</code>.
   </dd>
   <dt>tabs</dt>
   <dd>
      A read-only live-updating array of all <code>tabbox_tab</code>s inside of this tabbox. Though the array itself is read-only, tabs inside of it can be modified.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.tabbox.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.tabbox.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>