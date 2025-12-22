
# UI polishing

## Imagespace Modifier

* Should we allow the user to unset properties on the first or last keyframe?

* It'd be nice if the seek slider could show tick marks for all defined keyframes.

  This is harder than it sounds. `QSlider::paintEvent` uses private variables to set properties on the `QStyleOptionSlider` instance used to control painting. This means that we can't subclass `QSlider` and repurpose `tickPosition`/`setTickPosition`; we'd have no way to intercept the original paint call and prevent the vanilla (evenly-spaced) tick marks from being drawn.
  
  Theoretically we could use an event filter to intercept the paint event from the outside, and paint additional tick marks. However, `QSlider` also adjusts its size depending on whether the vanilla tick marks are enabled.
  
  I think, then, that the only way to achieve this would be to subclass `QSlider`, intercept `sizeHint` so that we always make space for our custom tick marks, and have custom tick marks be defined with properties other than `tickPosition` and friends. You'd leave the vanilla tick marks disabled when using the widget.

* Once we have imagespace support working in the Render Window, we should have the "Show in render window" checkbox influence whether the imagespace is visible at all, and add an additional "TEST" button that plays the imagespace's animation (if any), as in the CK.
