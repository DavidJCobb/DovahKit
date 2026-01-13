
Debugging Qt can be really annoying when 80% of the library uses the PIMPL idiom and therefore MSVC and its debugger can't see inside 80% of its data structures.

I got desperate enough to actually start creating Natvis files that use horrible pointer hacks to let me see inside some of them, as my situations required. Qt 5's source code is online, so if I'm determined enough and I also hate myself, I can basically reconstruct the class layouts in the jankiest way imaginable.

