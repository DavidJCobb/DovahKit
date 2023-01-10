
# ui.widget

Base class for all UI controls. Additionally, when used directly, it functions as a generic container for other controls.

Instances of <code>widget</code> can contain other widgets. This property does not extend to instances of any subclass.

## Instance methods

<dl>
   <dt>instance:add_child(child, row, col, rowspan, colspan)</dt>
   <dd>
      <p>Moves <var>child</var> inside of <var>instance</var>, positioning it according to the latter's layout, if any.</p>
      <p>The <var>row</var>, <var>col</var>, <var>rowspan</var>, and <var>colspan</var> arguments are all optional, and default to 1. If any are specified, however, they must be positive non-zero integers. The purpose and function of these arguments depends on <var>instance</var>'s layout, if any.</p>
      <p>This function throws an error under the following conditions:</p>
      <ul>
         <li><var>instance</var> is of a widget type that cannot contain other widgets</li>
         <li><var>child</var> is not a widget</li>
         <li><var>child</var> and <var>instance</var> are the same widget</li>
         <li><var>child</var> currently contains <var>instance</var>, whether directly or indirectly</li>
         <li><var>child</var> is a special widget that cannot be reparented</li>
         <li>any of <var>row</var>, <var>col</var>, <var>rowspan</var>, or <var>colspan</var> are specified incorrectly</li>
      </ul>
   </dd>
   <dt>instance:add_spacer(type, row, col, rowspan, colspan)</dt>
   <dd>
      <p>Adds a flexible "spacer" object inside of <var>instance</var>. The <var>type</var> must be a case-insensitive string; the following values are allowed:</p>
      <dl>
         <dt>both</dt>
         <dd>
            The spacer expands and contracts on both axes.
         </dd>
         <dt>h</dt>
         <dt>horizontal</dt>
         <dd>
            The spacer expands and contracts horizontally.
         </dd>
         <dt>v</dt>
         <dt>vertical</dt>
         <dd>
            The spacer expands and contracts vertically.
         </dd>
      </dl>
      <p>The <var>row</var> and <var>col</var> arguments are optional unless the widget is using a grid layout. The <var>rowspan</var>, and <var>colspan</var> arguments are optional. All optional arguments among these four default to 1. If any are specified, however, they must be positive non-zero integers. The purpose and function of these arguments depends on <var>instance</var>'s layout, if any.</p>
      <p>This function throws an error under the following conditions:</p>
      <ul>
         <li><var>type</var> is not a string, or is an unrecognized string</li>
         <li><var>instance</var> is of a widget type that cannot contain other widgets</li>
         <li>any of <var>row</var>, <var>col</var>, <var>rowspan</var>, or <var>colspan</var> are specified incorrectly</li>
      </ul>
   </dd>
   <dt>instance:can_have_layout()</dt>
   <dd>
      Returns true if this widget is of a type that is allowed to contain other widgets, or false otherwise.
   </dd>
   <dt>instance:get_layout_stretch_at(a, b)</dt>
   <dd>
      <p>Attempts to check the stretch factor at a given position in the widget's layout. Returns the stretch factor as an integer, or returns nil if the widget has no layout or the specified position is out of bounds.</p>
      <p>The meaning of the arguments varies depending on the widget's layout type:</p>
      <dl>
         <dt>grid</dt>
         <dd>
            <var>a</var> is a case-insensitive string indicating an axis ("row", "col", or "column"), and <var>b</var> is a positive non-zero integer indicating a position along that axis.
         </dd>
         <dt>h</dt>
         <dt>ltr</dt>
         <dt>rtl</dt>
         <dd>
            <var>a</var> is a positive non-zero integer indicating a column, and <var>b</var> is ignored and unused.
         </dd>
         <dt>v</dt>
         <dt>down</dt>
         <dt>up</dt>
         <dd>
            <var>a</var> is a positive non-zero integer indicating a row, and <var>b</var> is ignored and unused.
         </dd>
      </dl>
   </dd>
   <dt>instance:on(event_name, listener_name, listener)</dt>
   <dd>
      <p>Registers a function, <var>listener</var>, to run whenever the specified event occurs on this widget. Different widget types support different events. The combination of event name and listener name uniquely identifies the listener, and can be used to remove it later. Attempting to register two listeners with the same event and listener names will cause the later listener to replace the older listener.</p>
      <p>Throws an error if the event or listener name arguments are not strings, or if the listener argument is not a function.</p>
   </dd>
   <dt>instance:remove_child(child)</dt>
   <dd>
      <p>Removes <var>child</var> from <var>instance</var></p>
      <p>This function throws an error under the following conditions:</p>
      <ul>
         <li><var>instance</var> is of a widget type that cannot contain other widgets</li>
         <li><var>child</var> is not a widget</li>
         <li><var>child</var> and <var>instance</var> are the same widget</li>
         <li><var>child</var> is not a child of <var>instance</var></li>
         <li><var>child</var> is a special widget that cannot be reparented</li>
      </ul>
   </dd>
   <dt>instance:remove_event_listener(event_name, listener_name)</dt>
   <dd>
      <p>Unregisters any previously-registered event listener with the specified names. Has no effect if no listener is associated with the specified combination of names.</p>
      <p>Throws an error if the event or listener name arguments are not strings.</p>
   </dd>
   <dt>instance:set_layout(type)</dt>
   <dd>
      <p>If this widget is of a type that can contain other widgets, then this method sets the widget's layout type, which determines how contained widgets are positioned; otherwise, this method throws an error. The <var>type</var> argument is a case-insensitive string and must be one of the following values:</p>
      <dl>
         <dt>none</dt>
         <dd>
            No attempt is made to arrange child widgets.
         </dd>
         <dt>grid</dt>
         <dd>
            Child widgets are arranged in a grid. When adding a child widget, you can specify row and column numbers, as well as the "rowspan" and "colspan," allowing a widget to span across multiple rows or columns.
         </dd>
         <dt>h</dt>
         <dt>ltr</dt>
         <dd>
            Child widgets are arranged horizontally from left to right. When adding a child widget, you can specify a column number and a colspan.
         </dd>
         <dt>rtl</dt>
         <dd>
            Child widgets are arranged horizontally from right to left. When adding a child widget, you can specify a column number and a colspan.
         </dd>
         <dt>v</dt>
         <dt>down</dt>
         <dd>
            Child widgets are arranged vertically from top to bottom. When adding a child widget, you can specify a row number and a rowspan.
         </dd>
         <dt>up</dt>
         <dd>
            Child widgets are arranged vertically from bottom to top. When adding a child widget, you can specify a row number and a rowspan.
         </dd>
      </dl>
   </dd>
   <dt>instance:set_layout_stretch_at(...)</dt>
   <dd>
      <p>Attempts to modify the stretch factor at a given position in the widget's layout. Throws an error if the widget is of a type that can't contain other widgets, or if the specified stretch factor is not a positive-or-zero integer.</p>
      <p>The arguments vary depending on the widget's layout type:</p>
      <dl>
         <dt>grid</dt>
         <dd>
            The first argument is a case-insensitive string indicating an axis ("row", "col", or "column"). The second is an integer indicating a position along that axis. The third is the stretch factor.
         </dd>
         <dt>h</dt>
         <dt>ltr</dt>
         <dt>rtl</dt>
         <dd>
            The first argument is an integer indicating a column, and the second argument is the stretch factor.
         </dd>
         <dt>v</dt>
         <dt>down</dt>
         <dt>up</dt>
         <dd>
            The first argument is an integer indicating a row, and the second argument is the stretch factor.
         </dd>
      </dl>
   </dd>
</dl>

## Instance properties

<dl>
   <dt>cursor</dt>
   <dd>
      <p>Can be used to override the cursor shown when the mouse pointer is hovered over this widget. If no override is active, then this property is nil. In addition to nil, the following values &mdash; all strings &mdash; are allowed; writing any other value throws an error.</p>
      <dl>
         <dt>busy in background</dt>
         <dd>
            A loading cursor shown when a program is busy but the bulk of its functions are still usable.
         </dd>
         <dt>crosshair</dt>
         <dd>
            A crosshair resembling a plus sign.
         </dd>
         <dt>forbidden</dt>
         <dd>
            A cursor indicating that you cannot perform a default action; typically displays as a "no" symbol.
         </dd>
         <dt>hand, closed</dt>
         <dd>
            A cursor resembling a hand, palm facing away from the viewer, fingers outstretched. Typically used when you're able to "grab" something by clicking and dragging, and are currently doing so.
         </dd>
         <dt>hand, open</dt>
         <dd>
            A cursor resembling a hand, palm facing away from the viewer, fingers outstretched. Typically used when you're able to "grab" something by clicking and dragging, but when you have yet to actually do so.
         </dd>
         <dt>normal</dt>
         <dd>
            The default mouse pointer.
         </dd>
         <dt>pointer</dt>
         <dd>
            A pointing hand; the cursor you see when you mouseover a hyperlink in a web browser.
         </dd>
         <dt>resize, h</dt>
         <dd>
            A horizontal resize cursor
         </dd>
         <dt>resize, v</dt>
         <dd>
            A vertical resize cursor.
         </dd>
         <dt>wait</dt>
         <dd>
            A loading cursor shown when a program is busy and inoperable.
         </dd>
         <dt>what's this?</dt>
         <dd>
            A cursor shown when activating "help" functions in many programs, to indicate that you can click on something to display an infobox about its function. Typically styled as a default mouse cursor but with a question mark alongside it.
         </dd>
      </dl>
   </dd>
   <dt>enabled</dt>
   <dd>
      A boolean value indicating whether the widget allows interactions. If false, the widget will generally be greyed out. This status applies transitively to all widgets inside of the current widget; however, this property only reflects whether the widget <em>itself</em> has been disabled <em>directly</em>, and not whether it is disabled indirectly as a consequence of being somewhere inside of another widget that is disabled.
   </dd>
   <dt>layout_margins</dt>
   <dd>
      <p>If this widget is of a type that cannot contain children, then reading this value produces nil, and attempting to write to it throws an error. Otherwise, reading this value will retrieve a table with both named ("top", "right", "bottom", and "left") and indexed properties indicating the amount of pixels by which the widget's children are inset from the widget's edges.</p>
      <p>You can write any of the following values to this property, though regardless of what format you set the margins in, they will always be retrieved in the format described above.</p>
      <ul>
         <li>A single integer, to be used as the margin value on all sides.</li>
         <li>A table <var>T</var> for which <code>#T == 1</code>, in which case <var>T[1]</var> will be used as the margin value on all sides.</li>
         <li>A table <var>T</var> for which <code>#T == 2</code>, in which case <var>T[1]</var> will be used as the top and bottom margin, and <var>T[2]</var> will be used as the left and right margin.</li>
         <li>A table <var>T</var> for which <code>#T == 3</code>, in which case <var>T[1]</var> will be used as the top margin, <var>T[2]</var> will be used as the left and right margins, and <var>T[3]</var> will be used as the bottom margin.</li>
         <li>A table <var>T</var> for which <code>#T &gt;= 4</code>, in which case <var>T[1]</var> will be used as the top margin, <var>T[2]</var> will be used as the right margin, <var>T[3]</var> will be used as the bottom margin, and <var>T[4]</var> will be used as the left margin.</li>
         <li>A table <var>T</var> for which <code>#T</code> is zero or non-numeric. <var>T.top</var>, <var>T.left</var>, <var>T.right</var>, and <var>T.bottom</var> will be used as the margins.</li>
      </ul>
      <p>If the first argument is neither an integer nor a table, then an error will be thrown.</p>
      <p>Negative numbers will be clamped to zero; non-integer numbers will be truncated to integers. If any margin is specified as a non-numeric value, then that margin will not be changed. If all margins are specified as unchanged, an error will be thrown.</p>
   </dd>
   <dt>max_height</dt>
   <dd>
      Gets or sets the maximum height for this widget. If no height limit is in effect, then this value is nil; otherwise, it is a positive non-zero integer. Writing an invalid value throws an error.
   </dd>
   <dt>max_width</dt>
   <dd>
      Gets or sets the maximum width for this widget. If no width limit is in effect, then this value is nil; otherwise, it is a positive non-zero integer. Writing an invalid value throws an error.
   </dd>
   <dt>min_height</dt>
   <dd>
      Gets or sets the minimum height for this widget. If no minimum height is in effect, then this value is nil; otherwise, it is a positive non-zero integer. Writing an invalid value throws an error.
   </dd>
   <dt>min_width</dt>
   <dd>
      Gets or sets the minimum width for this widget. If no minimum width is in effect, then this value is nil; otherwise, it is a positive non-zero integer. Writing an invalid value throws an error.
   </dd>
   <dt>name</dt>
   <dd>
      Gets or sets the internal name for this widget. Widgets have an empty name by default, do not require unique names, and do not display their names. This value must be a string; writing anything else, including nil, is an error.
   </dd>
   <dt>tooltip</dt>
   <dd>
      Gets or sets the widget's tooltip text, shown by the OS when the mouse hovers over the widget for long enough. Widgets have an empty tooltip string by default. This value must be a string; writing anything else, including nil, is an error.
   </dd>
   <dt>whats_this</dt>
   <dd>
      Gets or sets the widget's help text, shown by the OS when the user clicks on the widget while its containing window is in "What's This?" mode. Widgets have an empty help string by default. This value must be a string; writing anything else, including nil, is an error.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.widget.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.widget.new()</dt>
   <dd>
      <p>Creates and returns a new blank widget. Passing any arguments to this function will throw an error.</p>
   </dd>
</dl>