
# Use info flags

DovahKit stores use info bidirectionally on form stubs. Each stub has an "inbound" and "outbound" map o form IDs to `use_info_entry` structs. That struct is presented here:

```c++
namespace dovah {
   struct use_info_entry {
      struct flag {
         flag() = delete;
         enum type : uint8_t {
            parent_child    = 0x01, // the user-form is a child of the used-form
            //
            // The next flags are useful for unique and high-importance relationships between 
            // specific forms. These must be relationships that can only exist once; for example, 
            // a REFR can only have one base form. If the relevant (form_reference_t) is altered, 
            // the flag will be removed.
            //
            // If two relationships can be outbound from the same form but are mutually exclusive, 
            // that alone is not enough to distinguish them, because a form with malformed data 
            // could be loaded. For example, DIAL/BNAM and DIAL/QNAM are mutually exclusive by 
            // virtue of involving different form types, but a file with ill-formed data could 
            // contain a DIAL that points both subrecords at the same form, and so using the same 
            // flag for both subrecords could in that situation cause use info mismanagement should 
            // either subrecord be altered after load.
            //
            object_reference = 0x02, // REFR/NAME: the user-form is a reference and the used-form is its base form
            dialogue_branch  = 0x04, // DIAL/BNAM
            dialogue_quest   = 0x08, // DIAL/QNAM and DLBR/QNAM
            water_acti_type  = 0x10, // ACTI/WNAM: a water activator's water type
            template_actor   = 0x20, // NPC_/TPLT: the used-form is the user-form's template actor
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      //
      form_stub* other    = nullptr;
      uint32_t   refcount = 0;
      flags_t    flags    = 0;
   };
}
```

We can see that <dfn>use info flags</dfn> mark specific uses on a form that are one-to-one, allowing us to rapidly look up relationships between forms without having to fully load them. For example, a `REFR` form will only ever list one other form as its base form, so that gets to have a use info flag; and the benefit of this is that we can now find a ref's base form without having to fully load that ref into memory. However, a `REFR` could potentially use multiple linked refs (each with a different keyword), so none of those can have a use info flag. The reason for this limitation is that a using-form could contain multiple uses of some other form, and the flag indicates that exactly one of these uses has a special meaning; supporting multiple uses would require a count rather than a flag.

Now, the implementation of this concept shown above isn't terrible. However, the flags should be redesigned somewhat. We store the flags in a single byte, so we only get eight bits to work with; but many of these flags are specific to certain form types. If we had consciously designed with that in mind, it would've been clean enough to overlap bits. Consider a design like this:

```c++
//
// The enums use bit indices, not masks, as the values. All flags are 
// named in terms of the user-form, not the used-form, unless otherwise 
// stated. For example, if there's a flag named "discombobulated_by," 
// then the used form is the discombobulator and the user form is what 
// gets discombobulated by the used form in-game.
//
namespace use_info_entry_flags {
   enum class base {
      parent = 0, // explicitly for "my record is in a child GRUP of this other form's record"
      
      __COUNT
   };
   constexpr const size_t first_form_type_specific_flag = (size_t)base::__COUNT;

   enum class activator {
      water_type = first_form_type_specific_flag, // ACTI/WNAM
   };
   enum class actor_base {
      template_actor = first_form_type_specific_flag, // NPC_/TPLT
   };
   enum class cell {
      encounter_zone = first_form_type_specific_flag, // CELL/XEZN
      location, // CELL/XLCN
   };
   enum class dialogue_branch {
      parent_quest = first_form_type_specific_flag, // DLBR/QNAM
   };
   enum class encounter_zone {
      location = first_form_type_specific_flag, // ECZN/DATA+0x04
   };
   enum class reference {
      base_form = first_form_type_specific_flag, // REFR/NAME
      loc_ref_type,     // REFR/XLRT
      persist_location, // REFR/XLCN
   };
   enum class topic {
      parent_quest = first_form_type_specific_flag, // DIAL/QNAM
      parent_branch, // DIAL/BNAM
   };
   enum class worldspace {
      encounter_zone = first_form_type_specific_flag, // WRLD/XEZN
      location, // WRLD/XLCN
   };
   
   template<dovah::form_type>
   using by_form_type = /*...*/;
   
   template<typename FlagType>
   constexpr dovah::form_type form_type_of = /*...*/;
}

namespace form_stub_helpers {
   template<auto Flag> /*`requires` clause to ensure we use a value from a valid flags type*/
   dovah::form_stub* get_used_form_with_flag(const dovah::form_stub& stub) {
      constexpr const auto mask = (decltype(use_info_entry::flags))1 << (unsigned int)Flag);
      if constexpr (!std::is_same_v<decltype(Flag), use_info_entry_flags::base>) {
         if (stub.form_type != use_info_entry_flags::form_type_of<decltype(Flag)>)
            return nullptr;
      }
      for(auto& pair : stub.outbound) {
         auto& info = pair.second;
         if ((info.flags & mask) != 0)
            return info.other;
      }
      return nullptr;
   }
}

// replacement for things like `base_form_reference_t`
template<auto Flag, auto AllowedFormTypes = std::array<form_type, 0>>
   /*`requires` clause to ensure we use a value from a valid flags type*/
class flagged_form_use {
   public:
      /* #if _DEBUG, setters should verify that the using-form is of the correct type for the flag */
};
```

Given that we have one flag which is used across all form types, every form type gets up to 7 type-specific flags, with the involved boilerplate (i.e. checking form types on stubs) abstracted away.

It's worth noting that structure padding means that technically, `use_info_entry` has three bytes of padding at the end. We may as well just make `flags_t` a `uint32_t`, meaning we have the full complement of 32 bits to work with, with 31 available for form-type-specific cases.

One last note: the enums for flags **must not** be `#include`d by the headers for form stubs or use info entries. We want to be able to add more of these enums, or add flags to these enums, without forcing 99.9% of the program to recompile. The `flags` field on `use_info_entry` should be a bare integral type e.g. `uint32_t`. Non-member accessors, which take a `dovah::form_stub&` as input, should be used to read and manipulate the flag. (Bonus points if the enums can each have their own file, with templates like `by_form_type`, `form_type_of`, and `get_used_form_with_flag` acting on forward declarations of the enums.)

## Recommended flags

* All forms
  * **Parent/child record relationship.** Technically, only forms that can be children[^child-forms] need this. However, making this a form-type-specific flag would be burdensome within the backend (i.e. having to bifurcate all flag accesses by form type).
* ACTI (Activator)
  * **ACTI/WNAM (Water Type):** For rapidly checking whether refs need persistence. If the base form is an Activator with a water type, then the CK says it needs persistence.
* CELL (Cell)
  * **CELL/ECZN:** To optimize gathering a Location's contents.[^location-gather]
  * **CELL/XLCN:** To optimize gathering a Location's contents.[^location-gather]
* DIAL (Topic)
  * **DIAL/BNAM (Parent Branch):** Used when deleting the parent branch, as part of an option to automatically delete all dialogue forms therein. Also used by the `DIAL` UI to prevent adding multiple topics with the same subtype to the same branch/quest.
  * **DIAL/QNAM (Parent Quest):** Used when deleting the parent quest, as part of an option to automatically delete all dialogue forms therein. Also used by the `DIAL` UI to prevent adding multiple topics with the same subtype to the same branch/quest.
* DLBR (Dialogue Branch)
  * **DLBR/QNAM (Parent Quest):** Used when deleting the parent quest, as part of an option to automatically delete all dialogue forms therein.
* ECZN (Encounter Zone)
  * **ECZN/DATA+0x04 (Location):** To optimize gathering a Location's contents.[^location-gather]
* NPC_ (ActorBase)
  * **NPC_/TPLT (Template Form):** For rapidly gathering all template actors that influence the contents of a templated actor. Templating can be daisy-chained, and different properties can be inherited by each successive actor, so you may need to consult several templates to determine the final properties of some templated actor.
* REFR (ObjectReference)
  * **REFR/XLRT:** To optimize gathering a Location's contents.[^location-gather]
  * **REFR/XLCN:** To optimize gathering a Location's contents.[^location-gather]
* WRLD (Worldspace)
  * **WRLD/ECZN:** To optimize gathering a Location's contents.[^location-gather]
  * **WRLD/XLCN:** To optimize gathering a Location's contents.[^location-gather]

[^child-forms]: CELL, LAND, NAVM, REFR, and subclasses of REFR.

[^location-gather]: LCTN forms need to maintain lists of: all unique actors that use the location as their Persist Location; all "special refs" (i.e. anything with a LocRefType) placed in a cell for which the LCTN is the immediate containing location (as determined by the cell's Encounter Zone or Location, or those of its parent worldspace); and some similar odds and ends. Additionally, the LCTN UI needs to go a step beyond, and show all unique actors that are placed in one of the location's cells *or* that use it as their Persist Location. It'd be really, really nice to be able to gather this information using Use Info alone.

Incidentally, given that I had to do some searching to track some of these down, I should probably maintain a document listing the use info entry flags that are defined, and the reasons why they're offered.