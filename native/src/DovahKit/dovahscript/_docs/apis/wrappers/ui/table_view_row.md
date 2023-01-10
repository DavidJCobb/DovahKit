
# ui.table_view_row

A wrapper object representing a single row in a `table_view`.

## Instance properties

<dl>
   <dt>cells</dt>
   <dd>
      A read-only live-updating array of cells in this row. The cells themselves are not read-only.
   </dd>
   <dt>index</dt>
   <dd>
      The index of this row within the table view, as a positive non-zero integer. Read-only.
   </dd>
   <dt>text_color</dt>
   <dd>
      The default text color for cells in this row.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.table_view_row.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>
