
In order to manage use info, we have to retain form-to-form pointers in memory using a small data structure (currently called `form_reference_t`) which enforces special access requirements: you cannot modify the value of a `form_reference_t` (to clear it, or make it point to a different form) unless you provide its containing form (so we can update use info for both the referent and the referrer). When DovahKit was first designed, then, all form-to-form pointers were implemented as `form_reference_t`. We can call these "managed uses."

Now, for relatively simple form-editing UIs, this isn't a problem. Consider, for example, the case of editing a Shout, with a dialog box that has "OK" and "Cancel" buttons. You have six drop-downs: three for Words of Power, and three for Spells. Until you click "OK," we can track all of the state &mdash; all of the changes you've made &mdash; entirely within the states of those UI controls.

This breaks down, however, when we get to more complicated UIs. Consider the case of editing a Quest's stages. First, you've got a listbox on the left, to pick a stage by number. Then, for whichever stage you happen to be currently looking at, you've got textboxes and all sorts of other doodads for editing that stage's data. This means, however, that the UI *itself* can only store state for one quest stage at a time: the stage you're currently looking at. If you make changes to a quest stage, and then switch your view to another quest stage, then we need somewhere to stash the changes that you've made but haven't saved (since you haven't clicked "OK" yet).

There are two solutions to this problem:

* Make every such complex UI define its own data structures that exactly mirror the backend's data structures for forms, but without managed use info.
* Make it possible to clone the backend data for a form, disable use info management for the clone, and then later "commit" the clone all at once, updating the managed copy. You could think of this as checkout and commit, if you're familiar with version control systems.

I went with the latter solution. I was not, however, skilled enough with C++ templates to do it *well*, at the time. I was also unaware of some old-school preprocessor macro tricks that would've been tremendously helpful. In practice, all form data is accessed the same way, managed or unmanaged: even if you have an unmanaged copy of a form, you still have to follow all the conventions for working with managed use info data; the code paths that you're calling into just politely decide not to do anything. The result is that any UI code that has to touch unmanaged data is unnecessarily clunky and ugly. Lots of `foo.bar.set(foo, value)` instead of just being able to do `foo.bar = value`. Lots of cases where this flaw alone prevents us from just `ui::bind`ing a UI control directly to the data structure it edits, fire-and-forget (`DKFormListPane` is one example).

So here's a better solution for handling form data in the backend, using HeadParts as an example.

```c++
// Base class with useful typedefs inside.
template<bool Managed>
class FormDataBase {
   using use = std::conditional_t<
      Managed,
      managed_form_use, // new name for `form_reference_t`
      form_stub*
   >;
   
   using use_list = std::conditional_t<
      Managed,
      managed_form_use_list, // variation on `std::vector` with use info management boilerplate
      std::vector<form_stub*>
   >;
   
   // for nested structs that contain uses.
   // this same pattern can be extended to std::variant, etc..
   template<typename T>
   using struct_list = std::conditional_t<
      Managed,
      managed_struct_list<T<Managed>>,
      std::vector<T<Managed>>
   >;
};

// HeadPart data.
template<bool Managed>
class HeadPart : public FormDataBase<Managed> {
   private:
      enum class head_part_flag_indices {
         playable,
         male,
         female,
         is_extra_part = 4,
         use_solid_tint,
      };
      
   public:
      using head_part_flags = dovah::enum_from_flags<head_part_flag_indices, uint8_t>;
      
   public:
      struct {
         form_components::model_ts<Managed> model;
         form_components::papyrus<Managed>  papyrus;
      } components;
      localized_string name;
      head_part_type   type = head_part_type::misc;
      head_part_flags  flags;
      use              color;       // CNAM
      use              texture_set; // TNAM
      use              valid_races; // RNAM
      struct {
         std::string race;
         std::string tri;
         std::string chargen;
      } morphs;
      use_list extra_parts; // HNAM[]
};
```

So now, the HeadPart data that lives in the backend is a `HeadPart<true>`, which has fully managed use info. The UI, however, can generate a `HeadPart<false>` that lacks the use info machinery, using bog-standard pointers and `std::vector`s, and it will no longer have to go through all the use info machinery to change things. Instead of `myHeadPart.color.set(myHeadPart, otherColor)`, we can just do `myHeadPart.color = otherColor` whenever we're working with unmanaged HeadPart data, with the "OK" button then writing our unmanaged data into the managed form. ("A whole rewrite just for that?" Skyrim has over 120 different kinds of forms, and most of those have several references to other forms. Even the smallest bits of ugliness add up *fast.*)

(In practice, I'd want the template parameter to be a struct, so that it's easy to extend it with other configuration options, but a `bool` is enough to illustrate the primary benefit here.)

There's another thing we can improve with this. Right now, we have to define the following functions on each form data class:

| Function | Purpose |
| :- | :- |
| `load` | Load `this` form's data, given a decompressed record to read as a binary stream. |
| `_save_impl` | Save `this` form's data. |
| `generate_use_info` | Parse a decompressed record (as a binary stream) to extract use info for a form of the given type, during the initial load of all applicable files. Skip all irrelevant data. |
| `_clone_impl` | Overwrite a destination form's data with a copy of `this` form's data. Used both for duplicating forms, and for the "commit" part of the checkin/commit process. |
| `_sever_outbound_references_impl` | Given a to-be-deleted form, sever all uses of that form by `this` form. |
| `_clear_impl` | Reset `this` form to its defaults, clearing out all uses of all other forms. Most notably used on forms before they are deleted. |

The last three functions are just different variations on "do something for every piece of data in this data structure." In practice, we end up repeating ourselves a lot -- not copying and pasting code verbatim, but writing four pieces of code that *say essentially the same thing*: the data structure definition itself, which lists its contents; and then these three functions where we're manually writing code that applies to each of those contained things.

An [X macro](https://en.wikipedia.org/wiki/X_macro) can solve this problem. We write a macro with a list of fields in our struct, and define `visit_fields` and `visit_paired_fields` member functions that rely on a macro which, for each field, invokes some other macro. Then, based on these two things, we can define "sever uses of," "clear," and "clone" functions, along with a function that converts between managed and unmanaged representations. We've cut three repetitions down to one.

```c++
template<bool Managed>
class HeadPart : public FormDataBase<Managed> {
   private:
      enum class head_part_flag_indices {
         playable,
         male,
         female,
         is_extra_part = 4,
         use_solid_tint,
      };
      
   public:
      using head_part_flags = dovah::enum_from_flags<head_part_flag_indices, uint8_t>;
      
   public:
      struct {
         form_components::model_ts<Managed> model;
         form_components::papyrus<Managed>  papyrus;
      } components;
      localized_string name;
      head_part_type   type = head_part_type::misc;
      head_part_flags  flags;
      use              color;       // CNAM
      use              texture_set; // TNAM
      use              valid_races; // RNAM
      struct {
         std::string race;
         std::string tri;
         std::string chargen;
      } morphs;
      use_list extra_parts; // HNAM[]
      
   public:
      #pragma region Field-visit boilerplate
         #define FOR_EACH_FIELD(X) \
            X(components.model) \
            X(components.papyrus) \
            X(name) \
            X(type) \
            X(flags) \
            X(color) \
            X(texture_set) \
            X(valid_races) \
            X(morphs.race) \
            X(morphs.tri) \
            X(morphs.chargen) \
            X(extra_parts)
      
         template<typename Functor>
         void visit_fields(Functor&& functor) {
            #define INVOKE(member) functor(this->member);
            FOR_EACH_FIELD(INVOKE)
            #undef INVOKE
         }
         
         template<typename Functor, bool OtherManaged>
         void visit_paired_fields(HeadPart<OtherManaged>& other, Functor&& functor) {
            #define INVOKE(member) functor(this->member, other.member);
            FOR_EACH_FIELD(INVOKE)
            #undef INVOKE
         }
         
         #undef FOR_EACH_FIELD
      #pragma endregion
};

//
// in some other file:
//
template<typename T>
concept can_visit_fields = requires (T& x, T& y) {
   { x.visit_fields([](auto& field) {}) };
   { x.visit_paired_fields(y, [](auto& fx, auto& fy) {}) };
};

template<typename T>
concept is_optional = requires {
   typename T::value_type;
   requires std::is_same_v<T, std::optional<typename T::value_type>>;
};

template<typename T>
concept is_managed_list = requires(T& x) {
   //
   // ...
   //
};

namespace impl {
   template<can_visit_fields T, typename U>
   void sever_uses_of(T& user, U& value, dovah::form_stub& used) {
      if constexpr (is_optional<U>) {
         if (value.has_value())
            _sever_uses_of(user, *value, used);
         return;
      }
      
      if constexpr (std::is_same_v<U, managed_form_use>) {
         if (value == &used)
            value.set(user, nullptr);
         return;
      }
      if constexpr (std::is_same_v<U, dovah::form_stub*>) {
         if (value == &used)
            value = nullptr;
         return;
      }
      
      if constexpr (std::is_same_v<U, managed_form_use_list>) {
         value.remove_all_of(user, &used);
         return;
      }
      if constexpr (std::is_same_v<U, std::vector<dovah::form_stub*>) {
         std::erase(value.begin(), value.end(), &used);
         return;
      }
      
      if constexpr (can_visit_fields<U>) {
         value.visit_fields([&user, &used](auto& v) {
            sever_uses_of(user, v, used);
         });
         return;
      }
   }
}

template<can_visit_fields T>
void sever_uses_of(T& user, dovah::form_stub& used) {
   user.visit_fields([&user, &used](auto& field) {
      impl::sever_uses_of(user, field, used);
   });
}

// "clear" can be implemented similarly.

// For "clone," we'll use the "visit paired fields" function.
template<can_visit_fields T>
T* clone(T& src) {
   T* dst = new T;
   src.visit_paired_fields(*dst, [dst](auto& src_field, auto& dst_field) {
      using field_type = std::decay_t<decltype(dst_field)>;
      if constexpr (std::is_same_v<field_type, managed_form_use>) {
         dst_field.set(*dst, src_field);
      } else if constexpr (std::is_same_v<field_type, managed_form_use_list>) {
         dst_field.overwrite_all(*dst, src_field);
      } else if constexpr (std::is_same_v<field_type, form_stub*>) {
         dst_field = src_field;
      } else if constexpr (std::is_same_v<field_type, std::vector<form_stub*>>) {
         //
         // ...copy between vectors, accounting for `src` being (un)managed...
         //
      } else if constexpr (can_visit_fields<field_type>) {
         //
         // ...pretend I could be bothered to work out the recursion logic right now...
         //
      } else {
         dst_field = src_field;
      }
   });
}
```

The above is all off the top of my head, untested. I'll probably want to lab this out in a throwaway VS project to hash out the details, e.g. recursion for when nested structs in a form (e.g. `std::vector<Quest::Stage>`) must themselves be visited while visiting the form. Ideally, when you ask to visit all data in a form, your own functor shouldn't have to be recursive; it should just be *invoked* recursively by the "visit everything" function, i.e. the "visit everything" function should be what recurses.

## General
* Namespaces
  * `dovah::form_data` for loaded-form data classes
  * `dovah::form_components` for things like VMAD, etc.
  * `dovah::form_structs` for common bits and bobs like RGB color triplets
* Type-safe flags-masks all over the backend. It's trivial to do this with templates now, so the "non-strict enum inside a struct" trick isn't needed anymore, and moreover, the enum values can be flag indices rather than handwritten flag values.
* A template that takes an enum type and an underlying type, and in essence "overrides" the enum's underlying type. So for example, we could define enums like `dovah::dialogue::category` without a specific underlying type, and then have `cobb::enum_in<dovah::dialogue::category, uint8_t>` for form data that explicitly needs it to be a single byte. (Currently, enums always have to have the underlying type set so we can pass instances directly into the serialization functions (and if ever the same enum has different underlying types in different places, we'll have to read an appropriately-sized integral and then cast it into place).)
