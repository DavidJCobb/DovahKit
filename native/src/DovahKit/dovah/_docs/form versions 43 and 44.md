
Every record in  an ES[LPM] file has a version number attached to  it. With the 
release of Skyrim Special Edition, these  version numbers became relevant; much 
of the community is now aware of the  terms "form version 43" and "form version 
44." What are the differences between these, and how do they work?

Form version  44 came about  because of changes  to the ES[LPM]  file format in 
Skyrim Special Edition. Some of these  changes were necessary in order to allow 
for new features  and functionality, while  other changes were  simply made for 
developer convenience.  In the early days of Skyrim Special  Edition modding, a 
number of Skyrim Classic mods  were incorrectly  ported for Skyrim Special, and 
so form versions quickly received a lot  of attention. This has led to a lot of 
discoveries, but also to a lot of misconceptions.

This document aims to explain the mechanics behind the different form versions, 
and to dive  into how exactly things tend  to go wrong when version  43 data is 
incorrectly supplied to a game that  expects version 44 data. I examine how the 
game handles  data in relatively close detail, and though I try to  explain the 
more  technical concepts  in a way that a non-programmer  can understand,  that 
does mean that the explanations get rather  long. Accordingly, let me lead with 
a quick summary.

**Version 43  data can occur in a valid  Skyrim Special Edition  file even when 
that file is not a  ported mod, and  there are situations  where it's perfectly 
safe for  that data to appear. Even Bethesda's official Skyrim Special  Edition 
content often  contains version  43 data. You should  not assume  that a mod is 
defective or dangerous just because it contains version 43 forms.**

Now, if you don't mind an essay-length explanation, let's begin.


## What actually changes from one version to another?

The most obvious  changes are when Skyrim Special Edition adds  new features to 
an existing record type, or even entirely  new record types, such as form types 
for lens flares. These cases are quite straightforward: SSE-specific data would 
be entirely meaningless  to a Classic file and would typically  be skipped, and 
because  such data can't naturally occur in a Classic file, incorrect  ports of 
such data are  not a concern. A mod author could certainly  handcraft incorrect 
data of these types, but that's not a problem specific to form versioning.

Much more common  are file format changes made for developer  convenience. It's 
common for Bethesda to load data structures  by blindly copying them out of the 
file,  such that their representation  in the file format essentially  is their 
representation in-memory. There are specific  cases where these data structures 
needed to have their layouts changed in order for this style of loading to work 
in Skyrim  Special Edition. The easiest  way to understand the nature  of these 
changes is to look at these data structures (or "structs") in something similar 
to their original  form: Skyrim and Skyrim  Special Edition  were programmed in 
C++ and were compiled with  the Microsoft Visual C++  (MSVC) compiler, so let's 
look at structs in C++ syntax.

I'll warn you real quick: we're going to be diving into technical details here. 
I'll do my best to try and explain everything so that non-programmers can grasp 
it all, but you're going to have to  remember a lot of information. There are a 
few things I need to explain, and only after I've explained them will I be able 
to connect them into a nice and neat whole.

Now, then. A basic struct looks like this:

```c++
struct MyStruct {
   uint8_t  falmer;
   uint16_t dwemer;
   uint8_t  goblin;
   uint32_t horker;
};
```

That struct consists of:

* A single-byte integer named "falmer"
* A two-byte integer named "dwemer"
* Another single-byte integer, this one named "goblin"
* A four-byte integer named "horker"

C++ and were compiled with  the Microsoft Visual C++  (MSVC) compiler, so let's 
The struct  provides the types and names of each data value. (If you're curious 
about how the types are indicated, they're C++ names: `u` for  "unsigned" (i.e. 
non-negative);  `int` for "integer;" and then the number of bits  the value has 
i.e. the number of bytes times eight).

So now that we know roughly what a struct looks like, let's explore a few ideas 
about them.


### Alignment and padding

Let's start with  one of those questions that seems simple on  the surface, but 
actually has some hidden depth. You know, the kind of question where there's an 
intuitively obvious answer, but it's going to be wrong somehow, and you're only 
being asked the question as a pretext for teaching you something.

How many bytes does the `MyStruct` struct above contain?

Well,  it has a one-byte  number, a two-byte number,  a one-byte number,  and a 
four-byte number. 1 + 2 + 1 + 4 = 8. The  struct has eight bytes worth of data, 
right? So the struct must be eight bytes long.

Nope.

See, modern CPUs are able to work with  values more efficiently when the values 
are *aligned*. A value is "*n*-byte aligned"  if the value's location in memory 
is a multiple of *n*. It's easier for a CPU to access a value that is *n* bytes 
long if that value is *n*-byte aligned;  so, a two-byte value is easier to work 
with if it's two-byte aligned, for example. And see, C compilers know this, and 
try to produce the most efficient code possible; that means that most compilers 
will actually add "padding" to a struct in order to align the individual values 
in that struct.

The struct itself will commonly be  four-byte aligned or eight-byte aligned, so 
all we  need to know  is the *offset* of its data values  &mdash; that  is, the 
distance  from the start of the struct to the start of a data value.  If a data 
value's offset is a multiple of the data value's size, then the data value will 
be properly aligned.

Here's what our `MyStruct` struct will  look like if it's run through a typical 
compiler.  I've taken the liberty of  annotating each data value  in the struct 
with its offset.

```c++
struct MyStruct {
   uint8_t  falmer;  //  0
   uint8_t  PADDING; //  1 // padding byte, to align the next field
   uint16_t dwemer;  //  2 // two bytes long and two-byte aligned
   uint8_t  goblin;  //  4
   uint8_t  PADDING; //  5 // padding byte, to align the next field
   uint8_t  PADDING; //  6 // padding byte, to align the next field
   uint8_t  PADDING; //  7 // padding byte, to align the next field
   uint32_t horker;  //  8 // four bytes long and four-byte aligned
   // End.           // 12
};
```

We can see that the "dwemer" field is two bytes long, and the field's offset is 
a multiple  of two; similarly, the "horker"  field is four bytes  long, and its 
offset is a multiple of four. The  "padding" bytes are just meaningless filler. 
It turns out  that `MyStruct` is twelve  bytes long, despite  only having eight 
bytes of meaningful data. And indeed, if you look through Skyrim's file format, 
you'll find several data structures that have this kind of padding; if you look 
closely, you may even be able to spot how the other values are aligned.

But what does this have to do with form  versions? There's another piece of the 
puzzle that we need to cover.


### How do form IDs work?

Let's start with a sentence that won't make a lot of sense if you're not really 
into programming, and then let's make it make sense: "Every pointer between two 
forms in memory is a union of a form ID and a pointer." And now that we've said 
all those nonsense words, let's dive into what a "union" is.

Typically,  in programming languages  like C++, a data value can  only have one 
type. "This  variable is an integer, it  will always be an integer,  and it can 
only be an integer. This other variable  is a decimal number, it will always be 
a decimal number, and it can only be a decimal number. This third variable is a 
piece of text, it will always be a piece of text, and it can only be a piece of 
text." But what if you needed to have a  value that can be one of any number of 
different things, depending on the circumstance?

That's when you use a union:

```c++
union MyUnion {
   uint32_t integer;
   double   decimal; // "double" means "double-precision." don't worry about it.
};
```

Here, we've  defined a new kind of union called `MyUnion`. If you  create a new 
variable that uses `MyUnion`, then that  variable can hold an integer or it can 
hold a decimal number. There are two things we need to know about unions:

* **A union  can only hold one value at a time, and is as large as  the largest 
  value it can hold.** This also means that  any variables using the union need 
  to be aligned  to the  size of the union.  Our `MyUnion`  can be a  four-byte 
  integer or an eight-byte "double," or double-precision decimal number, so the 
  union always  needs to have  eight bytes' worth of  room, and it  needs to be 
  eight-byte aligned.

* **A union doesn't actually hold any  information about its type.** If we have
  a `MyUnion`  variable  somewhere, we  don't automatically  know  whether that 
  particular variable has an integer or  a decimal number in it. We may have to 
  design our  program so that we can  infer the type based on  where it's being 
  used or what else is happening, or we  may need to design our program to keep 
  track of the type by hand.

So this brings  us back to our original  statement: "Every  pointer between two 
forms in memory is a union of a form ID and a pointer." We can restate this as, 
"In every place  where you would expect  the game to use a  pointer between two 
forms, the game actually uses a union of a four-byte form ID and a pointer."

A "pointer," of course, is just a way that two objects can refer to each other. 
Every object in a computer's memory has a "memory address," kind of like houses 
have street addresses, and these memory  addresses are numbers; so, the easiest 
way for one object  to refer to another  is for the former  object to store the 
memory address of the latter object. If one form needs to refer to another, the 
referring form will store a pointer to  the referred-to form, once the game has 
loaded both of the forms.

But what happens if the game loads the  referring form *before* the referred-to 
form? The referred-to form hasn't been loaded yet, so it won't be in memory; it 
won't *have* a memory address. This is  why Bethesda used a union. Skyrim loads 
game data in  two steps: for  the first step, it  loads all  forms into memory, 
storing form IDs in the places where  a pointer would normally go; and then for 
the second step,  it goes over all those  forms and converts  the form IDs into 
pointers.

Normally, using a union is harmless. Skyrim was originally designed as a 32-bit 
game,  and on 32-bit  systems, a pointer is four bytes  (32 bits), just  like a 
form ID. On a 64-bit system, however,  a pointer is eight bytes (64 bits). This 
means that Bethesda's form-ID-and-pointer  union is four bytes larger in Skyrim 
Special than it is in Skyrim Classic. Of course, this shouldn't cause any weird 
things to happen for mods, right? I mean,  it's not like Bethesda ever just has 
the game copy data directly out of a file and into memory&mdash; Oh.


### Bethesda sometimes just has the game copy data directly out of a file and into memory

Yeah, remember that thing I said about eighteen paragraphs ago? There are a lot 
of data structures that Bethesda loads by  just blindly copying them out of the 
file and into memory. It's not nearly *all*  of the game's data structures, but 
it's several of them, and importantly, they don't seem to have changed that for 
Skyrim Special.

So now we understand why the way a  struct is laid out actually matters when we 
have to deal with Skyrim Classic and  Skyrim Special. Let's lay out an example:

```c++
union FormPtr {
   TESForm* pointer; // "TESForm" is the game's internal name for "forms"
   uint32_t form_id;
};

struct SomeMadeUpData {
   FormPtr  some_other_form;
   uint8_t  aggression;
   uint8_t  suggestion;
   uint16_t confusion;
};
```

In Skyrim Classic, the `SomeMadeUpData` struct will have a four-byte form ID, a 
one-byte  "aggression" value,  a one-byte  "suggestion"  value, and  a two-byte 
"confusion" value. Conveniently, we don't actually need any padding here.

*However*, in Skyrim Special, `FormPtr`  needs to be eight bytes, even if we're 
only actually using it to store a four-byte form ID. That means that for Skyrim 
Special, the  `SomeMadeUpData` struct will have a four-byte form  ID along with 
four bytes of padding, to make room for  an eight-byte pointer; then we'll have 
our aggression, suggestion, and confusion values.

This means that if you were to take a  `SomeMadeUpData` made for Skyrim Classic 
and try to have Skyrim Special load it,  Skyrim Special would actually skip the 
aggression, suggestion,  and confusion values: it would  think that they're the 
meaningless padding within the eight  bytes reserved for the form ID and memory 
pointer. The game just ate four bytes.

This is the heart  of the form version issue. To my knowledge,  the majority of 
the file  format changes  going from version 43 to version  44 are  just struct 
layout changes stemming from the fact that 64-bit systems have larger pointers.


## Reasoning about these changes

Now that we actually understand the  most common file format difference, we can 
reason about it.  The game has well over a hundred and twenty  form types, so I 
don't have exact examples  memorized and I don't  think anyone has ever stopped 
to write  up a full list. Let's imagine a simplified example instead  of trying 
to find a real-world one:

```c++
struct ExampleStruct {
   FormPtr  some_form;
   uint32_t configuration;
   uint8_t  foo;
   uint8_t  something;
   uint16_t whatever;
   uint8_t  bar;
};
```

If we encode that struct for form version 43, such that `FormPtr` is four bytes 
long, and then  try to load it as form version 44, where  `FormPtr` is expected 
to be eight bytes long, then a few things happen:

* The `configuration` value is skipped  entirely: it's mistaken for the padding 
  within `FormPtr`.

* Similarly, everything else is shifted back by four bytes, so the `foo` value, 
  the `something` value,  and the `whatever` value are all  mashed together and 
  treated as a `configuration` value.

* As part of everything being shifted  back, the `bar` value is read as a `foo` 
  value.

So what's going to happen in-game? Well,  it depends entirely on what this data 
is actually used for, and how it's used.  The game may or may not perform error 
checking when it reads  the values, or  when it uses  the values. Moreover, the 
data that the  game actually ends up with might not even be  invalid; certainly 
it won't be what  the mod author intended, but if the  value that they intended 
for `bar` happens to be valid for `foo`,  then the `foo` value, at least, won't 
cause any game corruption because it *is* a value that the game allows.

Ultimately, it's  the same as if you were  to use xEdit or  a hex editor to rip 
open a file and stuff garbage values into it.

Now, let's  look at a slightly  more complicated  example, with  Skyrim Classic 
offsets noted:

```c++
struct AlignExample {
   uint32_t stuff;  //  0
   FormPtr  form;   //  4
   uint32_t things; //  8
   uint32_t foo;    // 12
   uint32_t bar;    // 16
};
```

Remember that whole  explanation we had about alignment?  Well, in this struct, 
the `FormPtr` field is four-byte aligned. That's fine for Skyrim Classic, since 
`FormPtr` is four bytes long in that game,  but in Skyrim Special, `FormPtr` is 
eight bytes long. That means that in Skyrim Special, the struct looks like this 
instead:

```c++
struct AlignExample {
   uint32_t stuff;   //  0
   uint32_t PADDING; //  4
   FormPtr  form;    //  8
   uint32_t things;  // 16
   uint32_t foo;     // 20
   uint32_t bar;     // 24
};
```

So if we take a `AlignExample` struct that was made for Skyrim Classic, and jam 
it into Skyrim Special, here's what happens:

* The `form` value  that we wanted to  use is at offset 4, so  it gets mistaken 
  for the padding used  to align the form in Skyrim Special:  an entire form ID 
  disappears into the ether.

* The `things` value that we wanted to use is at offset 8, so that gets misread 
  as a form ID.

* The `foo` value we wanted to use is  at offset 12, so Skyrim Special mistakes 
  it for the padding at the end of the form ID, and skips it.

* The `bar` value we wanted to use is at offset 16, so Skyrim Special treats it 
  as the `things` value instead.

Again, the game behavior that results from this situation is going to depend on 
how the game  actually uses this data; however, there's also a  new wrinkle. We 
just tricked the game  into taking data  that extremely  *isn't* a form ID, and 
loading that data as a form ID. Form IDs are just big numbers, so the result of 
this situation is going to depend on whether the number we put there happens to 
match the ID of  a valid form. If it doesn't, then the game  is going to handle 
that as well  as it would  handle us intentionally  passing 0 as  a form ID: if 
we're allowed to *not* refer to a form, then it'll go fine; otherwise, the game 
will probably crash somewhere, when it tries to access a form that isn't really 
there. That specific situation shouldn't lead to any corruption.

Where things get weirder is the case where the number we passed in just happens 
to actually match  up with the ID of a valid form. In that  case, the game will 
treat this the same as if we had  intentionally referred to that specific form. 
What's going to happen? Well... it depends!  If that form is of the right type, 
then we'll  see some  unexpected behavior,  but we shouldn't  see any  wild and 
crazy corruption; if the game was expecting a quest and we accidentally give it 
a quest, then the game got what it wants, even if it's not what *we* want.

Worse is  the case where  the form ID we used  is for a form that  isn't of the 
right  type &mdash;  think of  the game expecting  a quest  and getting  a door 
instead.  If the game  chooses to double-check the form  type (and in at  least 
some cases, it  does), then it'll see that the form we gave it is  of the wrong 
type, and it'll just clear the pointer; this will have the same effect as if we 
intentionally chose  not to specify any  form. If we just happen  to screw up a 
struct where the game *doesn't* check  form types, then we're very likely going 
to get a crash  or some kind of data corruption; certainly at  this point we're 
in the  realm of what  programmers call  "undefined  behavior," which in  plain 
English means "all bets are off, buddy."

I'll disclose  that the lion's  share of my research  has been into  the game's 
first loading step,  where we initially  pull all of the  data into memory, and 
not the  second loading step, where we convert form IDs to pointers.  The first 
step  is where issues  with struct layouts and padding  would occur, while  the 
second step is where form type checking would be done. If you're trying to make 
your own  tools for working with game data, as I am, then there's  little value 
in researching  the second step in  great detail, so I can't  say how often the 
game actually does type-checking for  what we've been calling `FormPtr`s above.

But again, this is  essentially identical to what would happen  if you manually 
edited a file to have bad data in it.


## What about the official files?

So far, we've been engaging in a lot  of theory based on reverse-engineering. I 
and a few other  people have dug into  how the game loads  data, and we've made 
good progress on understanding it. But shouldn't we also look at Bethesda's own 
official content, and see what we can learn from it?

Well, as it happens,  the official Skyrim  Special Edition  content &mdash; the 
base  game and  bundled DLCs &mdash;  consists of a mixture  of version-43  and 
version-44 forms. Bethesda only converted  the specific forms that needed to be 
converted in order to either take advantage of new features, or work around the 
struct issues that we've discussed above.

The game loads  official forms and modded forms in the exact  same way. If your 
mod contains valid data in the same file format as Bethesda's own content, then 
you can expect the same behavior. It has been claimed in the past that Bethesda 
modified Skyrim Special Edition to provide  special version-43 handling for the 
official content only; this claim is nonsense and directly contradicts what has 
been learned about the overall form-loading process via reverse-engineering. If 
something is safe in official ES[LPM]s,  then it should be safe in modded files 
as well.


## Conclusions

We can draw a few conclusions, then:

* The vast majority  of Skyrim's data is identical between  versions 43 and 44. 
  If a given  piece of data is identical between the versions,  then it doesn't 
  matter whether the data is marked as being version 43 or version 44.

* If a form type uses version 43 in  Bethesda's official Skyrim Special Edition 
  files, then  it is safe for that form type to use version 43  in modded files 
  at least as long as  those forms in  that mod don't  contain some new kind of 
  data that Bethesda added to Skyrim Special Edition.

* If version  43 data occurs  in specific  places where  the game  is expecting 
  version  44 data, then  the data in question is,  essentially,  corrupt data. 
  This is essentially the same situation  as editing bad data into a file using 
  xEdit; version 43 data is a unique  way for this corruption to occur, but the 
  data is not uniquely corrupt per se.

* The precise negative consequences  of corrupt data depend on how that data is 
  being used. Crashes are actually the  best outcome; if you're lucky, the game 
  is crashing before it can cause any permanent damage.

Ultimately,  version 43  data is not  radioactive in  the way that  much of the 
community believes. There are many  situations where it is definitively safe to 
have version  43 data in files intended for use in Skyrim Special  Edition, and 
even when version 43 data is incorrect  and therefore corrupt, that data is not 
necessarily  guaranteed to  obliterate  someone's  game or  save files  (though 
obviously, I  strongly encourage mod authors to avoid shipping  corrupt data to 
their users, and I will  never say that  it's *safe* to  supply corrupt data to 
the game).

It's common for mod authors and users alike to use scripts that are intended to 
scan a load order and identify any mods  that contain any version 43 data; it's 
fortunately somewhat  less common, but still too common, for  users to react to 
this by immediately contacting the authors  of these mods and telling them that 
they must resave the mod in the Creation Kit in order to avoid certain disaster 
and keep from bringing blight upon the land. This is excessive.

If I must give concrete recommendations, they would be...

### For authors

* If you're  porting a mod from Skyrim  Classic to Skyrim Special,  please just 
  re-save it in the Creation Kit. Make  sure to install Nukem's CK Fixes first, 
  because there are actually a few pieces  of data that the public Creation Kit 
  forgets to convert properly, including some weapon data.

* If you've created  your content entirely  in SSE-supporting  tools, using SSE 
  data as a base, then you're probably fine. For example, if you use SSEEdit to 
  override data  in Bethesda's official ESMs that is version 43,  it's fine for 
  your override to be version 43 as well, so long as you're not adding new data 
  that takes advantage of SSE-only features  (e.g. if Bethesda were to add some 
  new subrecord, or some new format for an existing subrecord).

* If at any point you're worried that  you might cause issues for people if you 
  don't re-save your mod in the Skyrim  Special Creation Kit, then just re-save 
  it in the Creation Kit. Don't ship mods that you feel anxious about shipping.

### For users

* If you see a  version 43 override in a Skyrim Special Edition  mod, check the 
  version on the overridden  (master) record. If the  overridden record is from 
  official content  and it's also version  43, then it's  probably fine. If, on 
  the other  hand, a version  44 record is being  overridden with a  version 43 
  record, then  that's at  least a little weird, and  I wouldn't  fault you for 
  reaching out to the mod's author for clarification.

* If the mod in  question *isn't even a  port*, and if you  don't have specific 
  reason to believe that something is wrong *other than* the fact that a script 
  told you that the mod has *the bad number* in it, then don't panic.

* If you don't understand any of the  above, then you lack the expertise needed 
  to check whether a version 43 record is  likely to be harmful.

* Regardless of your expertise, if a version 43 record ever actually leaves you 
  feeling worried,  then just re-save the mod's ESP file in  the Skyrim Special 
  Creation Kit  (with CK fixes installed) yourself. Don't go  combing your load 
  order and bothering mod authors over it  unless you have a clear reason to be 
  concerned.