
# DKBreadcrumbBar

Modeled after the breadcrumb bar seen in Windows 10's Open/Save File dialog.

Known issues as of 12/8/2025:

* **Display of the current item's icon is not implemented.** The native breadcrumb bar shows the icon for the current item (i.e. the trailing segment in the bar) at the bar's leading edge, before the root-menu button. The icon is visible even when the textbox is open. Clicking on this icon pops the textbox. Mouseover on the icon counts as mouseover on the root menu but doesn't display hover styles on the root menu.
  * We should have a property on the widget which controls whether it makes room for an icon or not. If this property is `true`, then always reserve space for an icon even if the current item has no icon.
  * We could optionally also allow setting a default `QIcon` to be used for items that have no icon of their own. If we want to get real fancy, we could allow varying the default icon based on whether the item has any child items, and perhaps supply "folder" and "page" icons as defaults for the two cases.

* **Segment menus behave differently.** Segment menus in the native breadcrumb bar are not ordinary popup menus; they have a few differences:
  * Constrained to a maximum of 18 rows tall.
  * Scrolling is done via a scrollbar embedded at the side of the menu, *not* via scroll buttons at the top and bottom edges.

* **Root menu displays hover state differently in the native widget, if-and-only-if any leading segments are culled.** It displays hover state even while its menu is open, and uses blue hover colors -- both inconsistent with the hover graphics for segments in dark mode.
  * Probably not worth implementing this.

* No combobox behavior (i.e. "drop-down" button).
  * Possibly not worth implementing.
  * How would we even decide what items to show?

* No ability to embed additional buttons at the trailing edge (e.g. the "Refresh" button seen inside the bar in the Windows 10 filepicker dialog).

* Menu positioning is a crapshoot and will probably be inconsistent/incorrect on other platforms/`QStyle`s.
  * Qt doesn't give us the tools we need to do a better job.