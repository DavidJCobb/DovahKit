
# ui.dropdown

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a drop-down menu, also known as a "combobox."

Drop-down menus can be configured to automatically sort their contents alphabetically. This means that every item in the drop-down has two indices: the <dfn>logical index</dfn>, which is the position within the unsorted list; and, when sorting is enabled, the <dfn>proxied index</dfn>, which is the position within the sorted list that the user sees.

For example, given the following unsorted list:

   1  Ari  
   2  Chris  
   3  Brianna  
   4  Quigley  
   5  Lucrezia  

If the list were to be `sorted`, then the items would be displayed in the following order:

   1  Ari  
   2  Brianna  
   3  Chris  
   4  Lucrezia  
   5  Quigley  

Lucrezia's proxied index would be 4, matching her visible position in the list. However, her logical index would remain 5, and if she were selected, the script would be told that the selected index is 5.

## Events

### OnChanged

Fires when the selected item changes. The logical index of the selected item is passed as an argument.

## Instance methods

<dl>
   <dt>instance:append_item(item)</dt>
   <dd>
      <p>Creates a new drop-down item, and adds it to the end of the drop-down list.</p>
      <p>If <var>item</var> is a table, then it may contain any of the fields available on a <code>dropdown_item</code>. Otherwise, <var>item</var> will be used as the drop-down item text, if possible.</p>
   </dd>
   <dt>instance:clear()</dt>
   <dd>
      Removes and deletes all items in the drop-down menu, emptying it.
   </dd>
   <dt>instance:map_logical_index_to_proxy(index)</dt>
   <dd>
      Given a positive non-zero integer representing a logical index, this function returns an integer representing the corresponding proxied index. If the specified logical index is out of bounds, then this function returns nil.
   </dd>
   <dt>instance:map_proxy_index_to_logical(index)</dt>
   <dd>
      Given a positive non-zero integer representing a proxied index, this function returns an integer representing the corresponding logical index. If the specified proxied index is out of bounds, then this function returns nil.
   </dd>
   <dt>instance:remove_item(item)</dt>
   <dd>
      Removes the specified item from the drop-down menu. The <var>item</var> argument must be the positive non-zero logical index of an item, or a <code>dropdown_item</code> instance belonging to this drop-down.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.items</dt>
   <dd>
      An array of all items in the drop-down menu, as <code>dropdown_item</code> instances. Items are listed in logical order, not proxied order. The property itself is read-only, though the items can be modified.
   </dd>
   <dt>instance.selected_index</dt>
   <dd>
      The logical index of the currently selected drop-down item, or nil if there is no selection. This value is writeable, but only positive non-zero integers, or nil, are allowed.
   </dd>
   <dt>instance.selected_item</dt>
   <dd>
      The currently selected drop-down item, if any, as a <code>dropdown_item</code> instance; or nil otherwise. This property itself is read-only, though the item, if not nil, can have its own properties modified even if accessed through this property.
   </dd>
   <dt>instance.selected_text</dt>
   <dd>
      The text of the currently selected drop-down item, if any; or an empty string otherwise. This value is writeable, but accepts only a string; if no drop-down item matches the provided string, then the setter has no effect.
   </dd>
   <dt>instance.sorted</dt>
   <dd>
      A boolean controlling whether the drop-down menu is automatically sorted.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.dropdown.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.dropdown.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>