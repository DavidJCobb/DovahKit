
* The "Morph into" context menu item for widgets is completely hardcoded. You can't make it possible to e.g. morph a native `QComboBox` into a custom control that is combobox-like.

* When you double-click on buttons, checkboxes, etc., and get an inline text editor, that is hardcoded into Qt Designer. You can clumsily recreate it by extending a custom widget type's task menu, and manually spawning a text-editor widget with the appropriate setup. For this plug-in, we have `InPlaceTextEditor` for the editor and `InPlaceTextEditableWidget` as a subclass (for widgets that want to clamp the text editor bounds to the label bounds).

* The part of Qt Designer that lists all available widgets is called the "widget box." The icons that Qt Designer uses for its own widgets are 22x22px PNGs.