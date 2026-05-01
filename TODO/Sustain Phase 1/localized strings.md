
I don't like how we currently handle localized strings in form data. In particular, it's hard to make it so that a mod can easily copy the localized strings of a base file (e.g. so that a French mod author changing an apple's sale value doesn't cause shenigans for English mod users).

In particular, I wish we could have an editing UI similar to ReachVariantTool, wherein the form-editing dialogs show L-strings in your chosen language but there's a button you can click to pop a dialog box and edit each localization. (As an improvement over RVT, we should also have "Copy All" and "Paste All" buttons that copy into multiple MIME types, so you can paste into Notepad as plain text or paste via the button to copy all localizations from one L-string to another.)

I also wish we could optionally generate per-language string files for all languages that are actually defined in a mod, when saving.

There are a few challenges here:

* It'd be more work to maintain the string table in memory: we'd have to support modifying it, we'd have to figure out what to do with strings that become orphaned/unused, and so on.

* If two places use the same L-string, we'd need to give the player the option to separate them... which requires being able to track references from forms to L-strings, bidirectionally, similar to Use Info. The current backend isn't designed for this. We don't even have a metaprogramming-based way to know which form types can even *have* L-strings. (Offhand I know TESFullName and TESDescription use L-strings, but we don't implement those as components in DovahKit, so we can't build dynamic dispatch for them; and in any case, there are other non-component places that use L-strings, such as string-type game setting values.)

We don't have to solve this for Sustain Phase 1; we can give it its own phase later on after our big-ticket phases are done.

## Minor improvements

* Use separate types for ILSTRINGS, DLSTRINGS, and LSTRINGS (i.e. `localized_info_string`, `localized_desc_string`, and `localized_string`). Currently we just have the one type with constructor args, and those are easy to forget when setting up new form types.

## Broader plans
Originally written 9/7/2024.

We need to know what stubs are using a given localized string, both so we know when an l-string is safe to delete and so we can show varying UI to the user. We don't need to know much else. L-strings therefore need inbound use info, but form stubs only need use info outbound to L-strings so we can sever on the L-string side if the form is deleted; enlarging the form stub struct is a necessary evil but we may be able to lessen the impact with a custom container. Forms would need to build L-string use info alongside normal form-to-form use info (trivial), and would need the ability to sever all references to a to-be-deleted L-string. Additionally, `localized_string` and friends would need to work like `form_use` (currently `form_reference_t`).

UI for working with L-strings would be enabled either program-wide or per active file. When L-strings are disabled, the UI for editing an L-string works as it does presently: you just see a single textbox, and edit a single language, and we write that content directly into your ESP when saving. When L-strings are enabled for the active file:

* Form fields for editing L-strings are read-only but not greyed out, and have two associated buttons: Replace and Edit.

* The Replace button allows you to set the given field (e.g. item name) to any appropriate L-string in the file. (This is subject to the current string type, i.e. the differences between LSTRING, DLSTRING, and ILSTRING.)

* The Edit button would open a dialog similar to the localized string editor in ReachVariantTool. The buttons for this dialogue would be arranged vertically (rather than the usual horizontal for dialog-close action buttons) and would be labeled:
  * Change this string just in this one place
  * Change this string everywhere it is used
  * Cancel all changes to this string

  The first button would fork the current L-string ref (e.g. `QImage::detach`) while the middle would edit the L-string directly and so affect all inbound refs to it.

  Notably, these options are being designed for the scenario of you editing localized data in a new form that you're defining. When overriding a form, we'd necessarily have to duplicate its original string content into your file. We could in that scenario just automatically search for any exact-duplicate strings owned by your file, and have the new override share the first such string it finds; and then subsequent changes to the override's localized text would lead to the above decision to make.

Additionally, when saving an existing active file, we'd want to update any extant string files; when saving a new active file, we'd want to warn if its filename or those of any localization are taken; and when overriding a form from a localized master, when the user commits the form dialog, we'd want to use an existing L-string in the active file (if an L-string in the edited form is exactly identical for all languages) or create a new L-string (otherwise).

Lastly: we want a default language: if you only know three languages, you shouldn't have to copy and paste one language into nine textboxes to prevent missing string errors; instead, the string you write in the default language should be written to the other languages by default (QPlainTextEdit placeholder). Unfortunately, English has to be the default language: character sets vary by language and I believe ASCII may be the only commonality between them.


## Broader plans (4/30/2026)

We need use info between forms and l-strings. Since not all forms will contain l-strings, and since modded files won't use l-strings at all, form stubs shouldn't include that use info inline; it should be a pointer to an outbound use-info map. (Since forms can use l-strings but l-strings can't use forms, we don't need a pair of maps on each side.)

In addition to requiring use info, l-strings must be aware of what string file they were loaded from, and it must be possible to know what data file that string file is a sidecar for.

### When editing a form

If a form is loaded from a localized master and then flagged as edited, such that we add it to the active file, and if the active file is also localized, then we must duplicate all l-strings within that form, from \[the localized string file for] the file that supplied the winning override, to the active file. This means that we'll need a virtual member function on managed form data to loop over all l-strings inside of the form data, with this being invoked by `form_stub::set_edited`.

(The form rewrite I have planned would make things easier: when "committing" unmanaged data to a managed form, check if any localizable strings in the source data are localized, what file they're from, et cetera. If the source data contains a non-localized string, and the destination file is localized, then create a new l-string. If the source data contains an l-string from a master, then copy the l-string into the active file and replace the reference. If the source data contains an l-string from the active file, then no action needed. Since we're already processing data field-by-field, no need to invoke a vfunc on the form data to update all the l-strings.)

### General

I need to think about every operation one could perform on an l-string, *and* every option one could perform on a form [i.e. a thing that can refer to l-strings], and for each such action I need to consider and plan out what to do with the l-strings and how.
