
# ui.table_view

This is a subclass of `ui.widget`, and inherits all non-static members.

## Events

### OnChanged

Fired when the currently selected row/column/cell changes. Listeners receive the new selection as an argument.

## Instance methods

## Instance properties

<dl>
   <dt>alternate_row_colors</dt>
   <dd>
      A boolean controlling whether row background colors alternate.
   </dd>
   <dt>column_headers</dt>
   <dd>
      An array of strings containing the text of the table view's column headers. Reading this value returns a copy, not a live-updating reference to the original.
   </dd>
   <dt>columns</dt>
   <dd>
      A live-updating array of column wrappers. The array is read-only, though its contents are not.
   </dd>
   <dt>has_corner_button</dt>
   <dd>
      A boolean controlling whether the table view has a corner button &mdash; an empty cell in the top-left corner. Disabled by default.
   </dd>
   <dt>min_column_width</dt>
   <dd>
      The minimum width, in pixels, that any column can have; or nil.
   </dd>
   <dt>rows</dt>
   <dd>
      A live-updating array of row wrappers. The array is read-only, though its contents are not.
   </dd>
   <dt>selection</dt>
   <dd>
      An array of the currently selected rows, columns, or cells, depending on the table's selection type.
   </dd>
   <dt>selection_mode</dt>
   <dd>
      <p>A string controlling how selections work. The following values are allowed:</p>
      <dl>
         <dt>single</dt>
         <dd>
            The user is allowed to select a single item.
         </dd>
         <dt>toggle</dt>
         <dd>
            The user is allowed to select a single item.
         </dd>
         <dt>multiple</dt>
         <dd>
            The user is allowed to select multiple items.
         </dd>
         <dt>contiguous</dt>
         <dd>
            The user is allowed to select multiple items, but the selection cannot have gaps.
         </dd>
         <dt>disabled</dt>
         <dd>
            The user can't select anything in the table.
         </dd>
      </dl>
   </dd>
   <dt>selection_type</dt>
   <dd>
      <p>A string controlling what the user can select. The following values are allowed:</p>
      <ul>
         <li>cells</li>
         <li>columns</li>
         <li>rows</li>
      </ul>
      <p>The default is <code>"rows"</code>.</p>
   </dd>
   <dt>show_column_headers</dt>
   <dd>
      A boolean controlling whether column headers are displayed.
   </dd>
   <dt>show_grid</dt>
   <dd>
      A boolean controlling whether borders are drawn between cells.
   </dd>
   <dt>show_row_headers</dt>
   <dd>
      A boolean controlling whether row headers are displayed. The default is <code>false</code>.
   </dd>
   <dt>sortable</dt>
   <dd>
      A boolean controlling whether the user can sort the table by any given column, by clicking the column header.
   </dd>
   <dt>word_wrap</dt>
   <dd>
      <p>A string controlling how text behaves when it reaches the edge of its containing cell. The following values are allowed:</p>
      <dl>
         <dt>none</dt>
         <dd>
            Text does not word-wrap.
         </dd>
         <dt>truncate</dt>
         <dd>
            Text does not word-wrap, but is truncated with an ellipsis.
         </dd>
         <dt>wrap</dt>
         <dd>
            Text will word-wrap onto multiple lines within a cell.
         </dd>
      </dl>
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.table_view.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.table_view.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>