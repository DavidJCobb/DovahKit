
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
namespace dovahkit::subsystems::load_order {
   class form_change_signals : public QObject, public cobb::singleton {
      Q_OBJECT;
      signals:
         void formDeletionImminent(dovah::form_stub&, bool just_being_flagged);
         void formDeletionComplete(dovah::form_id, bool just_got_flagged);
   };
   
   class core : public cobb::singleton {
      protected:
         dovah::active_load_order _load_order;
         
      public:
         void delete_form(dovah::form_stub& stub) {
            auto& emitter = form_change_signals::get();
            auto  form_id = stub.formID;
            emit emitter.formDeletionImminent(stub, false);
            //
            // ...
            //
            emit emitter.formDeletionComplete(form_id, false);
         };
   };
}
```

That actually works. Qt signals are literally just functions that, when called, invoke signal handlers; and if those functions aren't made protected or private, then anything can just... call them.

This will allow us to add new accessors to the subsystem without forcing a rebuild of every source file that needs the signals.