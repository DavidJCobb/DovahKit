
# ui

DovahKit's UI library.

## Methods

<dl>
   <dt>ui.run_when_locked(functor)</dt>
   <dd>
      Queues a function, <var>functor</var>, to run as soon as possible, locking the UI while the function runs: the user will be blocked from interacting with the UI in any way that would trigger a scripted UI event. (For example, buttons and textboxes won't be disabled, but button clicks will be ignored, and attempts to focus or type in a textbox will fail silently.) Throws an error if <var>functor</var> is not a function.
   </dd>
   <dt>ui.run_when_unlocked(functor)</dt>
   <dd>
      Queues a function, <var>functor</var>, to run as soon as possible. Throws an error if <var>functor</var> is not a function.
   </dd>
</dl>

## Properties

Every scriptable UI control has a property on the `ui` object: a singleton that can be used to construct an instance of the control, e.g. `ui.window.new("Title")`. Refer to each control's article for information on these.