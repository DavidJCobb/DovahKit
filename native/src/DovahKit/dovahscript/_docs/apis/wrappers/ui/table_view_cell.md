
# ui.table_view_cell

A wrapper object representing a single cell in a `table_view`.

## Instance properties

<dl>
   <dt>alignment</dt>
   <dd>
      A string controlling the alignment of text within the cell.
   </dd>
   <dt>column</dt>
   <dd>
      The cell's containing column. The property is read-only, though the column itself is not.
   </dd>
   <dt>font</dt>
   <dd>
      The <code>font</code> used for the cell's text.
   </dd>
   <dt>icon</dt>
   <dd>
      A color, <code>dds_resource</code>, or <code>raster</code>.
   </dd>
   <dt>row</dt>
   <dd>
      The cell's containing row. The property is read-only, though the row itself is not.
   </dd>
   <dt>text</dt>
   <dd>
      The text displayed in the cell.
   </dd>
   <dt>text_color</dt>
   <dd>
      The color of the cell's text.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.table_view_cell.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>
