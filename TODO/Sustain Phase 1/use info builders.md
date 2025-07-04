
# Use info builders (UIBs)

A simple enough struct. Each form type defines a `generate_use_info` static member function, which takes a record reader and a "use info builder" object. The function reads the record data as appropriate, and calls `add_outbound_reference` on the UIB, to add seen form IDs into a vector. Afterwards, code outside of the form type will `commit` the UIB.

Some care is required when handling form IDs:

* If a subrecord adds a form ID to some list inside of the form data, then you can add the seen form ID as it is read: `if (subrecord.read(form_id)) uib.add_outbound_reference(form_id);`.

* If a subrecord *sets* a form ID (i.e. the bulk of form uses), then you shouldn't add outbound references in real time. Instead, add the outbound reference only at the end of use info generation.

## Subordinate UIBs

Some form uses may be discarded during load. For example, DovahKit stores Papyrus attachment data for aliases on the aliases themselves; and so if `QUST/VMAD` contains Papyrus data for a non-existent alias, we want to discard that data and any use info it would produce. To that end, given a normal UIB, it's possible to spawn a "subordinate" UIB and read pending form IDs into *that*. Then, when you're done with it, you call `commit` on the subordinate UIB.

**This is fragile.** Nothing's stopping you from spawning a subordinate of a subordinate. However, there's no functional difference between a normal UIB and a subordinate. If you `commit` a subordinate, you aren't transferring its to-be-added form IDs to its parent; you're just finalizing the subordinate UIB, in exactly the same way that the top-level UIB is finalized outside of form-type-specific code. **This sucks, and is bad. It should be changed in a redesign.**

Subordinate UIBs are seldom necessary, and as of this writing, they're never nested. However, this is still a footgun; the system still sucks and should be changed.

In general, discarded-during-load uses weren't considered during the earliest stages of DovahKit's design, and that's a huge failing.