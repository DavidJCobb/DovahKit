
# UI polishing

## General

* `DKConditionList`, when disabled, doesn't allow resizing of its columns. Instead of disabling the listview, would it be possible to disable the child items in the listview, and the listview's root item, such that the widget is mostly non-interactable but the columns are still resizable?

* Make it possible to enable sorting on a `DKFormListPane`. Allow sorting by editor ID or form ID, with extra-columns sorting by `Qt::DisplayRole`. We especially want this for the list panes that show Locations' contents, in the LCTN UI.

* I forgot to implement having `DKBreadcrumbBar` show the icon of the current QMI, though that's moot right now since the idle model doesn't define any icons. I should implement a property to enable/disable showing icons, layout code to reserve space for an icon when icons are enabled, and code to fetch and cache the icon for the current item, and to update that on `dataChanged`.

* Anything that loads filesystem icons should do so via `QFileIconProvider`. I believe currently, for the BSA browser UI, we use WinAPI directly.

## Imagespace Modifier

* Should we allow the user to unset properties on the first or last keyframe?

* It'd be nice if the seek slider could show tick marks for all defined keyframes.

  This is harder than it sounds. `QSlider::paintEvent` uses private variables to set properties on the `QStyleOptionSlider` instance used to control painting. This means that we can't subclass `QSlider` and repurpose `tickPosition`/`setTickPosition`; we'd have no way to intercept the original paint call and prevent the vanilla (evenly-spaced) tick marks from being drawn.
  
  Theoretically we could use an event filter to intercept the paint event from the outside, and paint additional tick marks. However, `QSlider` also adjusts its size depending on whether the vanilla tick marks are enabled.
  
  I think, then, that the only way to achieve this would be to subclass `QSlider`, intercept `sizeHint` so that we always make space for our custom tick marks, and have custom tick marks be defined with properties other than `tickPosition` and friends. You'd leave the vanilla tick marks disabled when using the widget.

* Once we have imagespace support working in the Render Window, we should have the "Show in render window" checkbox influence whether the imagespace is visible at all, and add an additional "TEST" button that plays the imagespace's animation (if any), as in the CK.
