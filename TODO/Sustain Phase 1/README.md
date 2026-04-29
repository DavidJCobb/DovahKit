
# Sustain Phase 1

I've been working on DovahKit for most of the last five years. I've gotten a lot better with C++ and with Qt over that time period, and I've learned from the many mistakes I've made in how I went about the development process. Unfortunately, that means that while DovahKit *works*, and works pretty darn *well*, the code isn't as clean or as maintainable as I'd like. There's a lot of boilerplate that results from misapplied abstractions &mdash; from me getting something *almost* right, such that it gets the job done but is *just* wrong enough to be painful to work with.

Accordingly, Sustain Phase 1 is basically going to be me rebuilding the program piece by piece, starting with the innermost parts of the backend. It's not going to be a rewrite or a redesign. DovahKit is going to work more-or-less the same way, with more-or-less the same internal architecture, doing more-or-less the same things. Really, it's just going to be a mass reorganization and renaming. Probably 80% of Phase 1 is going to be copying, pasting, and rearranging.

That rebuild should implement the following form types first:

* Action
* Activator
* ActorBase
* Book
* Camera Path
* Dialogue Branch
* Furniture
* Idle
* Material Object
* Package
* Quest
* Story Manager (Branch|Event|Quest) Node
* Topic
* TopicInfo

Those form types will put most or all of the systems DovahKit needs through their paces:

* Conditions (IDLE, INFO, PACK, QUST)
* Form-trees (AACT, CPTH, IDLE)
* Formn types that are subclasses of each other (ACTI -> FURN)
* Localized strings (ACTI, BOOK, DIAL, FURN, INFO, NPC_, QUST)
* NIF export support (MATO, NPC_)
* Quest aliases (QUST)
* Papyrus
* Saving accessory files automatically (QUST -> SEQ files)
* Saving accessory files on demand (NPC_ -> facegen)
* UIs that reuse a single window for all forms of a given type (CPTH, IDLE)
* UIs that host a single form, but manage editing of multiple pseudo-descendant forms (QUST -> DLBR/DIAL/INFO, SMEN -> SMBN/SMQN)
