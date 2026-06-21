
# Dividing up `DovahKitCore` and `file_load_order`

The backend and frontend both have some nasty god objects:

* The backend has `file_load_order` (which I intend to rename to `active_load_order` during sustain), which stores and owns all loaded files, all loaded BSAs, all form stubs, and all loaded game settings.

* The frontend has `DovahKitCore`, a singleton which manages all access to the `file_load_order`. This alone makes it a hideous god-object-by-proxy, but additionally, all signals regarding forms being created, renumbered, modified, or deleted run through this singleton.


## Breaking up the backend

We need an overarching `active_load_order` object that manages and coordinates all the data  we have to load. However, we can still split that data into smaller components, even if these components remain somewhat coupled.

* `tes_file_collection`
* `form_stub_collection`
* `game_setting_collection`


## Breaking up the frontend

We've already been building separate "subsystems" for the frontend where possible, e.g. for tracking Papyrus scripts, for the form-info-cache, and for Worldedit and Worldinput. We can split `DovahKitCore` into subsystems such as:

* Active Load Order subsystem
* Game Setting subsystem
  * We actually already have a subsystem for this, but it just wraps `DovahKitCore` under the hood.

This isn't the most helpful if 98% of the UI still hooks signals like `formModificationImminent`, `formModified`, `formDeletionImminent`, and so on, though. There's a potential solution to that too, though: make the signal emitters be separate singletons.

Consider the following.

```c++
// each of these signal singletons gets its own header
namespace dovahkit::form_change_signals {
   class deletion : public QObject, public cobb::singleton {
      Q_OBJECT;
      signals:
         void imminent(dovah::form_stub&, bool just_being_flagged);
         void complete(dovah::form_id, bool just_got_flagged);
         
      public:
         template<typename Functor>
         void dispatch(dovah::form_stub& stub, bool just_being_flagged, Functor&& f) {
            auto id = stub.form_id;
            emit imminent(stub, just_being_flagged);
            f();
            emit complete(id, just_being_flagged);
         }
   };
}

// and this gets its own header too
namespace dovahkit::subsystems::load_order {
   class core : public cobb::singleton {
      protected:
         dovah::active_load_order _load_order;
         
      public:
         // implementation and `#include`s of signal singletons would be in `.cpp` file.
         // implementation is just shown here for illustrative purposes.
         void delete_form(dovah::form_stub& stub) {
            bool just_being_flagged = /* ... */;
            form_change_signals::deletion::get().dispatch(stub, just_being_flagged, [&stub]() {
               // ... delete the stub here ...
            });
         };
   };
}
```

That actually works. Qt signals are literally just functions that, when called, invoke signal handlers; and if those functions aren't made protected or private, then anything can just... call them.

This will allow us to add new accessors to the subsystem without forcing a rebuild of every source file that needs the signals.

### "Form modified" signals and calling out specific modifications

For certain optimizations, including ones we're likely to want for Worldedit in the future, we may want to make it possible to emit "form modification" signals that carry information about the specific form data that has changed.

**Do not implement any of this until after reading the "Criticisms of this draft" and "A possible compromise" subsections.**

#### Ideas

Obviously we don't want to have to define enums, etc., for every possible piece of form data that *can* change, however. We also want to strike a good balance between *not* having to allocate that kind of information on the heap with every emission, and *not* having to construct large-ish empty structs when we don't emit that kind of information. So, we should probably make it so that that signal is emitted with a const pointer argument (default to null) to a list of changes:

```c++
namespace dovahkit::form_change_signals {
   class modification : public QObject, public cobb::singleton {
      Q_OBJECT;
      public:
      
         // Identifies the frontend system that caused the change. Allows systems 
         // to ignore changes that they themselves have made, while still taking 
         // note of any changes made by other frontend systems.
         using change_source_type = cobb::eight_cc;
      
         // Information on specific changes being made to a form. If you're only 
         // changing one or two properties, and any frontend systems benefit from 
         // specifically knowing that, then you can emit the signal with a list 
         // of these.
         struct change {
         
            // identifies the kind of thing being changed.
            // 
            // bespoke for each form type; it is not this subsystem's job to care about this.
            // maybe good to define some `constexpr` constant ones in individual header files; 
            // and maybe even helper functions to generate the appropriate info for some desired 
            // variety of form data change (e.g. "landscape edit," "navmesh cover change," etc.).
            cobb::eight_cc signature;
            
            // sufficient for e.g.: a list index; a LAND quad index, x, and y; etc.
            // should describe what/where the change is, not the actual data that was previously 
            // there or is being written there.
            std::array<int64_t, 3> info = {0};
            
         };
         
      signals:
         void imminent(dovah::form_stub&, change_source_type = 0, size_t change_count = 0, const change* = nullptr);
         void complete(dovah::form_stub&, change_source_type = 0, size_t change_count = 0, const change* = nullptr);
         
      public:
         template<size_t N> requires (N > 0)
         void imminent(dovah::form_stub& s, change_source_type cs, const std::array<change, N>& changes) {
            emit imminent(s, cs, changes.size(), changes.data());
         }
         
         template<size_t N> requires (N > 0)
         void complete(dovah::form_stub& s, change_source_type cs, const std::array<change, N>& changes) {
            emit complete(s, cs, changes.size(), changes.data());
         }
         
         // Convenience function: emit "imminent," call lambda to make changes, and then 
         // emit "complete."
         template<typename Lambda, size_t N> requires (N > 0)
         void dispatch(dovah::form_stub& s, change_source_type cs, const std::array<change, N>& changes, Lambda&& f) {
            imminent(s, cs, changes);
            f();
            complete(s, cs, changes);
         }
         
         // Convenience function: emit "imminent," call lambda to make changes, and then 
         // emit "complete."
         template<typename Lambda>
         void dispatch(dovah::form_stub& s, change_source_type cs, Lambda&& f) {
            imminent(s, cs);
            f();
            complete(s, cs);
         }
   };
}

//
// examples
//

namespace dovahkit::form_changes::landscape {

   namespace default_quad_texture {
      constexpr const cobb::eight_cc signature = 'QuadTxtr';
      
      static form_change_signals::change describe(uint8_t quad_index) {
         assert(quad_index < 4);
         return form_change_signals::change{
            .signature = signature,
            .info      = { quad_index }
         };
      }
   };

   // Use only for when a single layer has its LandTexture changed. If 
   // adding/removing LandTextures from the list, don't emit this.
   namespace layer_texture_single_changed {
      constexpr const cobb::eight_cc signature = 'TxtrSngl';
      
      static form_change_signals::change describe(size_t index) {
         return form_change_signals::change{
            .signature = signature,
            .info      = { index }
         };
      }
   };
   
   // Use for when multiple layers have their LandTextures changed, or 
   // when adding/removing LandTextures from the list.
   namespace layer_texture_list_changed {
      constexpr const cobb::eight_cc signature = 'TxtrList';
      
      static form_change_signals::change describe() {
         return form_change_signals::change{
            .signature = signature
         };
      }
   };

   namespace _impl {
      form_change_signals::change describe_vertex_edit(cobb::eight_cc signature, uint8_t x, uint8_t y) {
         assert(x < dovah::data::landscape::vertices_per_side);
         assert(y < dovah::data::landscape::vertices_per_side);
         return form_change_signals::change{
            .signature = signature,
            .info      = { quad_index, x, y }
         };
      }
      
      // poor man's namespace
      template<cobb::eight_cc Signature>
      struct vertex_attribute_change {
         vertex_attribute_change() = delete;
         ~vertex_attribute_change() = delete;
      
         constexpr const cobb::eight_cc signature = Signature;
         
         // Vertex positions are cell-relative; quads are irrelevant.
         static form_change_signals::change describe(uint8_t vertex_x, uint8_t vertex_y) {
            assert(x < dovah::data::landscape::vertices_per_side);
            assert(y < dovah::data::landscape::vertices_per_side);
            return form_change_signals::change{
               .signature = signature,
               .info      = { x, y, 0 }
            };
         }
      };
   }
   
   using vertex_alpha_paint = vertex_attribute_change<'VertAlph'>;
   using vertex_color_paint = vertex_attribute_change<'VertColr'>;
   using vertex_height      = vertex_attribute_change<'VertHite'>;
}

void some_function_that_changes_a_landscape(form_stub& landscape) {
   auto  data_ptr = _some_function_to_load_managed_data(landscape);
   auto& data     = *data_ptr;
   
   dovahkit::form_change_signals::modification::get().dispatch(
      landscape,
      'Wrldedit',
      std::array{
         dovahkit::form_changes::landscape::default_quad_texture::describe(2)
      },
      [&]() {
         // apply changes...
      }
   );
}
```

Some recommended change-source values:

| Eight-CC | System | Notes |
| :- | :- | :- |
| `Backend!` | Backend | The backend can automatically make changes to forms, e.g. to sever uses of none-stubs or to-be-deleted forms. The frontend subsystem that manages the active load order should be listening for these changes (the backend offers relevant callbacks) and emitting signals for them as appropriate. |
| `CPthWndw` | Camera Paths window |
| `Dovascpt` | Dovahscript |
| `FormWndw` | Form edit window | The edit dialog for the form being modified. In some cases, one form's data may be edited by another form's window (e.g. the Story Manager UI editing event conditions on a Quest form); those cases *should not* use this change-source. |
| `IdleWndw` | Idle Animations window |
| `Wrldedit` | Worldedit | Could be used to let Worldedit make changes to its internal state and to renderer state directly, when tools are invoked. |

Some recommended change signatures:

| Form type | Eight-CC | Data | Extra params |
| :- | :- | :- | :- |
| *\[all\]* | `EditorID` | Editor ID |
| *\[all base forms\]* | `IsMarker` | The "Is Marker" flag |
| *\[all base forms\]* | `ModlPath` | The NIF file path for the form's world model[^world-model] |
| *\[all base forms\]* | `ModlSwap` | The TextureSet swaps for the form's world model[^world-model] |
| `LAND` | `QuadTxtr` | Default texture for a quad | quad index |
| `LAND` | `TxtrList` | LandTexture(s) added/removed from layer list |
| `LAND` | `TxtrSngl` | LandTexture replaced for a single layer | layer index |
| `LAND` | `VertAlph` | Alpha of a vertex | cell-relative X/Y |
| `LAND` | `VertColr` | Color of a vertex | cell-relative X/Y |
| `LAND` | `VertHite` | Height of a vertex | cell-relative X/Y |
| `REFR` et. al | `BaseForm` | Base form |
| `REFR` et. al | `Trnsform` | Position, rotation, and/or scale |

[^world-model]: Where base forms can have multiple models, the term "world model" identifies the model used by refs of that base.


#### Criticisms of this draft

I like the idea of specifying the "change source."

Not sure how I feel about being able to list specific changes:

* For a form-edit dialog, we'd have to specify no changes (which signal receivers would treat as "potentially anything may have changed") unless we diff the form data before committing it. Those diffs would have to be form-type-specific, too.

* The implementation drafted above doesn't really define a difference between "no notable traits of this form have changed" versus "any notable traits of this form could have changed." There's no way to represent the former at all. This means that if we *were* to diff form data in form-edit dialogs (and any other "you can potentially edit anything in here" system), and no notable traits changed, we'd end up emitting an empty change list which would be interpreted as "anything may have changed;" the work of diffing would be wasted.

  * Could we define a sentinel change signature, e.g. `'0notable'`, to indicate that we are specifically saying that zero notable properties of the form have changed?
  
    * But then we'd still have to implement diffing, which feels like it'd be a pain in the neck. Not to mention that a diff would have to use e.g. a `std::vector` to hold the changes, since the number of changes made could potentially vary.

* A system like Worldedit cares about nearly all of the data on a `LAND`, `NAVM`, or `REFR`, and it has to maintain its own state pertaining to all of that data. For example, if ref A is linked to ref B, then Worldedit has to tell the renderer to draw a line from A to B whenever A or B is selected... which means that it can't just load A, create that line fire-and-forget-style, and then move on; Worldedit has to *remember* that A is linked to B and it has to associate that fact with a handle to some renderer state that entails drawing that line.
  
  So suppose the `REFR` UI diffs a ref and detects that its linked refs have changed, and then the "form modified" signal carries that information. Worldedit then has to loop over the list of changes and branch on the signature, to conditionally update what linked refs it's tracking and rendering. Wouldn't that be *more* overhead than just having Worldedit diff the data itself? Even if the changes are diffed for Worldedit, Worldedit still has to perform a lookup to find and update the state pertaining to linked refs, no? Contrariwise, if Worldedit is responsible for the diffing, then it can diff more selectively; for example, if we only display links on the selected ref (or perhaps on the selected ref and then transitively on any links of links \[of links \[of links \[...]]]), then Worldedit could check for linked-ref changes only on those refs for which it's actually interested in displaying links. (And if we want to be able to display inbound links to selected refs, then we can just check whether a changed ref has inbound use info to any of our selected refs, and if so, see if it's now linked to any of our selected refs. And so on.)
  
  Similarly, for Worldedit to efficiently process changes to base forms, it'll need to maintain bidirectional connections: a map of base forms to refs; and for each ref, a pointer to the last-known base form (so that when an individual ref is unloaded, the base-to-ref map can be updated). So Worldedit already needs to track the information necessary to check whether a ref has been switched to another base form, and thus it can diff that information itself; it doesn't need the info diffed for it, and nothing else in DovahKit really needs that diff either.
  
  So if Worldedit can distinguish signals about its own changes from signals about changes made by the outside world, then sure, Worldedit can use fast paths for its own changes: have its tools make the changes directly, have it update internal and render state directly, have it properly emit the signals for those changes but then ignore those signals when they "bounce back." The optional "change source" parameter on the signals is good. But I'm kind of struggling to see what benefit Worldedit gets from the list of "change signatures."

  * Similarly, there's a lot of data that Worldedit currently doesn't cache, but that it would need to cache in order to properly handle changes to forms (see `handling form changes.md` in the Worldedit plans). For example, we don't currently cache any information about landscapes; however, if we want to implement proper responses to modifications of a TextureSet form, then we'll need to cache that information (i.e. so we can tell which LandTextures used that TextureSet, and which Landscapes are thus influenced by the change). I'm struggling to think of cases where Worldedit *doesn't* (or *wouldn't*) already need to keep track of things on its own, outside of the renderer.


#### A possible compromise

My main concern is that I don't want to end up in a situation where I need to extend the signal with more information to optimize some subsystem, or otherwise to make something work, and then I have to recompile 90% of the program again (the exact problem I want to avoid by dividing the signals apart in the first place). At the same time, I don't want to commit to adding and designing in functionality when I'm not sure it's actually necessary. If for example Worldedit performs just fine with only a change source, and not a list of specific form data that has changed, then I don't want to have the latter functionality sitting around unused, nor do I want other code to be going to the trouble of sending that information just for it to go ignored.

So what about a compromise: a hook for future extensibility, without going all the way and implementing the extra info right off rip?

The signal singleton could use a forward-declaration of a non-nested "change info" type. That type only needs to be defined when it's sent and when it's consumed:

```c++
namespace dovahkit {
   class form_modification_signal_info;
}

namespace dovahkit::form_change_signals {
   class modification : public QObject, public cobb::singleton {
      Q_OBJECT;
      public:
      
         // Identifies the frontend system that caused the change. Allows systems 
         // to ignore changes that they themselves have made, while still taking 
         // note of any changes made by other frontend systems.
         using change_source_type = cobb::eight_cc;
         
      signals:
         void imminent(dovah::form_stub&, change_source_type = 0, const form_modification_signal_info* = nullptr);
         
      public:
         // Convenience function: emit "imminent," call lambda to make changes, and then 
         // emit "complete."
         template<typename Lambda>
         void dispatch(dovah::form_stub& s, change_source_type cs, Lambda&& f) {
            imminent(s, cs);
            f();
            complete(s, cs);
         }
         
         // Convenience function: emit "imminent," call lambda to make changes, and then 
         // emit "complete."
         template<typename Lambda, size_t N> requires (N > 0)
         void dispatch(dovah::form_stub& s, change_source_type cs, const form_modification_signal_info* changes, Lambda&& f) {
            imminent(s, cs, changes);
            f();
            complete(s, cs, changes);
         }
   };
}
```

We never have to actually define the `dovahkit::form_modification_signal_info` type. We can leave it undefined, and our code will always pass `nullptr` to the signal singleton. If, eventually, we decide that we *do* want to be able to emit some sort of specific information about what data was changed, then we can:

* Create a separate header to define the `dovahkit::form_modification_signal_info` type.
* Include that header in the code that wishes to emit a signal annotated with this specific information.
* Include that header in the code that wishes to check for that specific information upon receiving a signal containing said information.

Thus `dovahkit::form_modification_signal_info` becomes a means to extend the signal in the future, without forcing literally everything that emits or receives the signal to recompile. The future definition of `dovahkit::form_modification_signal_info`, if we ever need it, can literally just be:

```c++
namespace dovahkit {
   class form_modification_signal_info {
      public:
         struct change {
            cobb::eight_cc         signature;
            std::array<int64_t, 3> info = {0};
         };
   
      public:
         size_t        size = 0;
         const change* changes = nullptr;
   };
}
```

And that would allow for a stack-allocated or heap-allocated list of changes, able to vary in length as the situation requires, just like the draft much further above.
