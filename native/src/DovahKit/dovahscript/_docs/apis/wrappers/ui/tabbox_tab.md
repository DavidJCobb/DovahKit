
# ui.tabbox_tab

This is a subclass of `ui.widget`, and inherits all non-static members. These widgets are tabs within a `tabbox`.

These widgets can contain other widgets.

## Instance properties

<dl>
   <dt>tab_enabled</dt>
   <dd>
      A boolean controlling whether the tab header is enabled.
   </dd>
   <dt>tab_name</dt>
   <dd>
      A string containing the tab's displayed name.
   </dd>
   <dt>tab_tooltip</dt>
   <dd>
      A string containing the tab header's tooltip. The difference between this and the <code>tooltip</code> property is that <code>tooltip</code> would apply to the tab's entire body.
   </dd>
   <dt>tab_whats_this</dt>
   <dd>
      A string containing the "What's This?" text for the tab header.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.tabbox_tab.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>