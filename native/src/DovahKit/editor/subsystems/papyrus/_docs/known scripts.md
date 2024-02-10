
# Known scripts

A <dfn>known script</dfn> is a Papyrus script (i.e. class) that we know either exists or should exist, either because we found PEX files which define the script, because we found PEX files which specify the script's name as a superclass, or because any form's VMAD contains a BoundScript using the script's name.

A script ceases to be "known" when all PEX files defining it are deleted, when no other known scripts potentially (see below) subclass it, and when no forms have it attached.

We try to keep track of every known script's inheritance hierarchy, and we try to update the set of known scripts (and data about each known script) as loose PEX files change out from under us. To those ends:

* The Papyrus subsystem monitors the directory for loose PEX files, watching for changes.

* For each known script, we track class information as loaded from up to two PEX files: a winning archived PEX, and a loose PEX. If the loose PEX is deleted, we can quickly revert the class hierarchy back to that established by the archived PEX.

This implies that a known script's inheritance hierarchy can change, as a result of modifications to a loose PEX defining the script itself or any of its ancestor classes. It also implies that for each known script *X*, we track up to two potential superclasses: one from the archived PEX file for *X*, if any; and one from the loose PEX file for *X*, if any.

We support and optimize for the following use cases:

* Given a known script, traverse up its class hierarchy (e.g. to see if it derives from a particular other script).

* Given a known script, check what native form or alias types it is attachable to. (We intend to apply more specific constraints than the Creation Kit, only allowing attachments that would successfully bind without Papyrus log errors during play.)

The following use cases are technically achievable, but not necessarily optimal:

* Given a known script, enumerate its child or descendant classes.

  * Each known script keeps track of its *potential* subclasses. We do this because given some script `Foo`, if the loose file `Foo.pex` is created/edited/deleted, that may alter `Foo`'s inheritance hierarchy, which would by definition affect all descendant classes of `Foo`. To respond to the loose file change, we need to be able to scan all *potential* descendant classes of `Foo`, identify any known scripts that are currently *actual* subclasses of `Foo`, and treat them as updated.

    This is not as optimal a process as traversing upward: for each downward traversal step from `Foo` to a *potential* subclass `Bar`, we have to double-check whether `Bar` *currently* inherits from `Foo` before proceeding; and thus also for traversing further, from `Bar` to its potential subclasses. It's okay for downward traversal to be clunkier, because it's only available to facilitate responding to PEX changes, and those *shouldn't* occur especially frequently or in especially large numbers at a time.

## Why keep track of known scripts?

* When the user decides to attach a new script to a form, we need to present them with a list of all available scripts they can use.

  * Unlike the Creation Kit, we want to filter this list based on which scripts can actually be attached to the form in question. This requires awareness of each script's inheritance hierarchy, so we can see what native class (if any) it derives from.

* Given some Papyrus property whose type is `Foo` or `Foo[]`, and a Papyrus script named `Foo`, we need to know what native class (if any) `Foo` derives from, in order to...

  * Show the right editing UI for setting the value (i.e. different widgets for picking general forms, ObjectReferences, or quest aliases).

  * Filter the available selections to those forms that have `Foo` attached.

    * Tracking known scripts is not, in itself, enough for this; we also need an optimal way to know what forms have a given script (or any subclass) attached. We intend to use `DovahKitFormDataCache` for this, giving it refcounting pointers to `known_script`s.