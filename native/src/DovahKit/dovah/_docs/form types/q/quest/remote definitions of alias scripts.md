
# Remote definitions of alias scripts

The `VMAD` subrecord on quest forms can specify script data to attach to aliases. The thing is, `QUST/VMAD` doesn't just specify alias IDs; it specifies quest form IDs *and* alias IDs. This means that a quest can actually attach scripts to aliases on completely different quests, provided one can guarantee that the "recipient" quest will be loaded prior to the "donor" quest. (That is: the "recipient" quest's first `QUST` record must have been seen before the "donor" `QUST/VMAD` subrecord is loaded.) In-game testing confirms that this actually works perfectly; you can hand-edit data in xEdit to behave like this and It Just Works.

DovahKit has absolutely no ability to handle this, at all. If we encounter an instance of this occurring, we warn the user and skip/discard the affected data entirely. This is because DovahKit treats Papyrus attachment data as part of the form on which it is defined; that is, the Papyrus data "lives in" the "loaded form" data. DovahKit can't account for a situation where data belonging wholly to one form somehow lives inside of a record for a totally separate form.


## Are there ways to enable support for this?

### The basics: loading donated data at all

To start with, we need to establish some terms; let's go with <dfn>donor form</dfn> and <dfn>recipient form</dfn>. We'll say that the data that is applied to the latter by the former is <dfn>donated data</dfn>. We'll also use the verb <dfn>transplant</dfn> to refer to the process of modifying both forms, to formally move the donated data out of the donor and into the recipient (such that it no longer *is* "donated").

We'd have to bidirectionally track donor/recipient relationships; doing so via form-stub addenda would be appropriate, since this situation shouldn't be at all common. We'd want to find and remember these relationships as part of the general task of building use info.

We'd have to have a specialized `load_as_donor_for` function to be invoked on the donor, with the recipient passed as an argument. This function would read the donor record, ignoring everything except for data that is donated specifically to the given recipient. Then, the process of loading a form can involve checking whether that form is a recipient and, if so, loading donated data from each donor.

(We can't just use the donor's normal `load` function, because among other things, a file could contain cyclical donor/recipient relationships: two quests could each donate to *each other.* If we can only fully load a form, and not load just data it donates to a specific recipient, then this would create a situation wherein we can't load either form, because loading A depends on loading B which depends on loading A which...)

This also creates some additional complications:

* How would we expose any of this in the UI?

* What happens to data in a recipient if one of its donors is deleted? What should we do with the donated data?

* What happens to donated data if a recipient is deleted? What should we do with that data?

* If the donor and recipient are both initially defined in the same file, then donation is unstable: we don't make any guarantees about the relative ordering of any two records of the same type, and if we end up serializing the donor *before* the recipient, then I would expect the donated data to be lost in-game. Thus, when saving an active file that contains the base records of both a donor and its recipient, we would want to transplant the donated data, and then sever the donor/recipient relationship.
