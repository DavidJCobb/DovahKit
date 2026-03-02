
# Core subsystems

## Coordinator

This subsystem coordinates script execution: it owns the Lua state, the worker thread for script execution, and the task queues; it blocks scripted UIs from reacting to events whenever the script is busy; and it's the main thing that handles requests from the outside world (i.e. to run a script).


## Events

This subsystem manages scripted event listeners attached to GUI widgets.


## Lifetime

Some native objects are meant to have lifetimes influenced or wholly controlled by Lua scripts. This subsystem is responsible for managing these native objects' lifetimes.

In general, GUI widgets and related objects (e.g. `QButtonGroup`) are managed by this function. If a tree of widgets is visible, or if Lua has a reference to any widget or related object in that tree, then all widgets and related objects in that tree must be kept alive.


## Permissions

Early in Dovahscript's design, I had considered making it possible for scripts to opt into or opt out of permission to take certain actions, such as modifying form data. This would've affected, at minimum, things like UI messaging and warning text when force-stopping scripts. None of this was ever implemented.


## Resources

This subsystem manages certain "resource" objects that can be loaded and used by scripts, e.g. raster graphics, DDS textures, and binary blobs. It takes care of double-buffering resources, so that when a graphical resource is being displayed in a scripted GUI, a script can write to that resource without having to thread-lock. It also manages the lifetimes of these resources using RAII-based handles.


## Userdata

This subsystem handles the Lua userdata objects that are used to wrap native objects and sub-objects, providing scripts with access to said (sub-)objects.
