
# UI polishing

## General

* I built my own classes for handling XAudio2, for playing FUZ files (i.e. dialogue audio). I should avoid interfacing with XAudio2 directly in favor of going through Qt's Multimedia module and using `QMediaPlayer`. The only reason I'm not doing that now is because enabling or disabling Qt modules will force a rebuild of the entire program, and I don't want to deal with that.

* `DKConditionList`, when disabled, doesn't allow resizing of its columns. Instead of disabling the listview, would it be possible to disable the child items in the listview, and the listview's root item, such that the widget is mostly non-interactable but the columns are still resizable?

* `DKHeaderView` doesn't use `Q_OBJECT` in its definition, so we can't `qobject_cast` to it. Add that macro, and in general, double-check all custom widgets and widget-like classes for it.

* IIRC `DKHeaderView` flips the hell out when the total widths of all columns exceed the width of the listview. Can we implement some sort of middle ground, wherein columns flex to fill a large space, but -- if the flex shrink factors are insufficient to fit them within the listview width -- don't flex to fit a small space?

* Qt's approach to header resizing is really painful and makes it hard to predict which sizing handle you even need to drag to make a given column larger. The normal behavior in Windows is that if I grab the handle between columns A and B and drag it rightward, A grows larger and B grows smaller. Qt does some weird nonsense where I think it's always the handle to one side, that makes a column to the other side grow larger or smaller without affecting the adjacent column?
  
  What I'm saying is, I should study the Windows native behavior versus the Qt behavior, find out where they diverge, and implement the former. Possibly I can have `DKHeaderView` offer the Windows behavior as an option that can be enabled and disabled.

* Make it possible to enable sorting on a `DKFormListPane`. Allow sorting by editor ID or form ID, with extra-columns sorting by `Qt::DisplayRole`. We especially want this for the list panes that show Locations' contents, in the LCTN UI.

* I forgot to implement having `DKBreadcrumbBar` show the icon of the current QMI, though that's moot right now since the idle model doesn't define any icons. I should implement a property to enable/disable showing icons, layout code to reserve space for an icon when icons are enabled, and code to fetch and cache the icon for the current item, and to update that on `dataChanged`.

* Anything that loads filesystem icons should do so via `QFileIconProvider`. I believe currently, for the BSA browser UI, we use WinAPI directly.

* If `DKFormPicker` has an unnamed exterior cell force-included, it displays the cell's form ID. It should ideally display the cell's grid coordinates and the editor ID of its parent worldspace. This is relevant for the `DKFormPicker` used to pick cells within the `DKObjectReferencePicker`: if you pick an exterior ref in the Render Window, then an exterior cell gets force-included into the cell picker.

## Imagespace Modifier

* Should we allow the user to unset properties on the first or last keyframe?

* It'd be nice if the seek slider could show tick marks for all defined keyframes.

  This is harder than it sounds. `QSlider::paintEvent` uses private variables to set properties on the `QStyleOptionSlider` instance used to control painting. This means that we can't subclass `QSlider` and repurpose `tickPosition`/`setTickPosition`; we'd have no way to intercept the original paint call and prevent the vanilla (evenly-spaced) tick marks from being drawn.
  
  Theoretically we could use an event filter to intercept the paint event from the outside, and paint additional tick marks. However, `QSlider` also adjusts its size depending on whether the vanilla tick marks are enabled.
  
  I think, then, that the only way to achieve this would be to subclass `QSlider`, intercept `sizeHint` so that we always make space for our custom tick marks, and have custom tick marks be defined with properties other than `tickPosition` and friends. You'd leave the vanilla tick marks disabled when using the widget.

* Once we have imagespace support working in the Render Window, we should have the "Show in render window" checkbox influence whether the imagespace is visible at all, and add an additional "TEST" button that plays the imagespace's animation (if any), as in the CK.
