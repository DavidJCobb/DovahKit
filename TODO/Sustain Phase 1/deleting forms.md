
# Deleting forms

The way DovahKit handles the deletion of forms isn't currently optimal.

## The current way

You begin by creating a `form_deletion_request` targeted to the form you want to delete. There are only minimal options that you can configure here, and they're specific to certain use cases:

* **`delete_dialogue_children`:** If you're deleting a quest or dialogue branch, then we'll find and delete all child topics (and by extension, their infos).

At the time the form deletion request is created, it will automatically gather all forms that require deletion. For example, if you ask to delete a cell, the request will automatically gather all landscapes, navmeshes, and refs inside of that cell for deletion along with the cell. To-be-deleted forms are categorized in two ways:

* Forms defined in the active file can be wholly deleted from memory.
* Forms defined in a non-active file can only be flagged as deleted. When the active file is saved, we'll emit override records with the "deleted" flag set.

Once the request has been created, the caller can query the list of forms to be deleted. This is useful for displaying messaging to the user about which forms will be deleted, and what those forms' Use Info is. At this point, the request can be "committed" to carry out the deletion, or it can be canceled (by simply destroying the request without ever committing it).

Creation of a form-deletion request may throw an exception, if any of the to-be-deleted forms can't be deleted. (This will generally only occur if a to-be-deleted form can't be loaded, e.g. because its form type isn't yet implemented, or if a to-be-deleted form has an inbound use and the using form can't be loaded.) The "commit" step is not allowed to fail and never throws.

### Problems

1. Gathering of to-be-deleted forms happens when the request is created. If any forms are modified between creation and committing of the request, then the request can enter an invalid state: we may delete forms that no longer requre deletion (e.g. if a ref was moved out of the to-be-deleted cell), or we may fail to delete forms that have since come to require deletion (e.g. if a ref was moved into the to-be-deleted cell).

2. Form-deletion requests don't smoothly handle a variety of parent/child relationships that aren't built into the file format. Some forms specify a "parent" via a subrecord, rather than being stored in a child `GRUP` of the parent form's record, and these cases have to be handled manually by form-deletion requests. We manually handle topics (`DIAL/QNAM` and friends) but we don't handle a variety of other cases (e.g. `CPTH/ANAM`, `IDLE/ANAM`, `SM*N/PNAM`, etc.).

3. When we flag entire hierarchies of forms as deleted (because some "parent" or "ancestor" form is being deleted), we still sever uses between the forms in the hierarchy. This means that if we *do* handle a subrecord-based parent/child relationship by deleting children alongside their parent, we'll also be severing the parent from its child such that the child no longer *is* a child. This causes complications for systems outside of the backend that track these hierarchies (e.g. the backend-provided datastores for camera paths, idles, and the story manager).
  
  Solving this is more complicated than you might expect, because the precise parent/child hierarchies that come to exist can depend on factors external to any two forms. The "idles" datastore is a good example, given its wonky error handling and potential for degenerate hierarchies. Given any action or idle, determining what its descendant idle(s) are may require computation of *every* idle's hierarchy placement. (This also begs the question of what to do when a given idle is in multiple hierarchy placements at once, and we delete an ancestor of *one* such placement.)

## An alternate design

### Between construct/gather and commit

When we commit a form-deletion request, that act should be allowed to throw a single exception. Specifically, we should gather all forms again: if a form that we gathered when the request was constructed no longer needs deletion, then we can simply decline to delete it; but if any newly-gathered form *wasn't* gathered at the time the request was constructed, then we should throw an exception immediately before we proceed.

### Form-type-specific options

We should offer a set of typed options, e.g.

```c++
class form_deletion_request {
   protected:
      // deleting QUST deletes child SCEN, DLBR, DIAL
      // deleting DLBR deletes child DIAL
      struct dialogue_options {
         bool delete_descendants = true; // default TRUE since we can go by use info alone
      };
      
      // Assume that the backend can optionally maintain its own datastore of 
      // idles. It has to be told to build that datastore, and we will throw 
      // if you choose to delete an idle with any option that requres that 
      // the datastore be ready.
      enum class idle_tree_behavior {
         ignore,
         
         // Direct children of a to-be-deleted idle are moved to LOOSE. If 
         // deleting an action, its root idle is moved to LOOSE (if the root 
         // is multiply-placed, then `idle_generate_behavior` determines 
         // whether we touch it at all).
         move_children_to_loose, // requires datastore
         
         // Direct children of a to-be-deleted idle are deleted. If deleting 
         // an action, its root idle is deleted (if the root is multiply-
         // placed, then `idle_generate_behavior` determines whether we touch 
         // it at all).
         delete_all_descendants, // requires datastore
      };
      enum class idle_degenerate_behavior {
         // This enum is only used if the idle tree behavior is not "ignore."
      
         // If a descendant is multiply-placed and its canonical placement is 
         // as a descendant, then delete it.
         delete_if_canonical, // requires datastore
         
         // Always delete multiply-placed descendants even if their canonical 
         // placement is elsewhere.
         always_delete, // requires datastore
      };
      struct action_options {
         idle_tree_behavior       tree_behavior       = idle_tree_behavior::ignore;
         idle_degenerate_behavior degenerate_behavior = idle_degenerate_behavior::delete_if_canonical;
      };
      struct idle_options {
         idle_tree_behavior       tree_behavior       = idle_tree_behavior::ignore;
         idle_degenerate_behavior degenerate_behavior = idle_degenerate_behavior::delete_if_canonical;
      };
      
      // Assume that the backend can optionally maintain its own datastore of 
      // camera paths. It has to be told to build that datastore, and we will 
      // throw if you choose to delete a camera path with any option that 
      // requres that the datastore be ready.
      struct camera_path_options {
         bool delete_descendants = false; // `true` requires datastore
      };
      
      struct scene_options {
         bool delete_dialogue_action_topics = true;
      };
      
      using typed_options_variant = std::variant<
         std::monostate,
         action_options,
         dialogue_options,
         idle_options,
         scene_options
      >;
      
   public:
      template<form_type ft>
      using typed_options_type = ...; // conditional
      
      // ... other state and options ...
      typed_options_variant typed_options;
};
```

This should handle the problem with child-via-subrecord forms.

### Severing uses of deleted forms

Currently, when we're about to delete a form, we walk all using forms, load them, and invoke `sever_outbound_references_to` on the form. This calls the virtual member function `_sever_outbound_references_impl` on the loaded form. Form deletion is the only time we ever invoke these functions.

The functions' contract (as implied by the name) is that they *will* sever all uses of the given form. However, some forms deliberately disobey this by necessity due to form-type-specific jank:

* An idle needs to track all action-root-relevant `ANAM` subrecords across all of the idle's records. If told to sever uses of an action, if that action is defined in a non-active file (and so is only being flagged as "deleted"), it won't sever uses within the `ANAM` tracking.

* A location actually *will* sever non-active-file-related tracking, but maybe it probably shouldn't?

We should replace this with:

```c++
public:
   bool on_before_used_form_deleted(const form_deletion_info&, form_stub&);
protected:
   virtual bool _on_before_used_form_deleted(const form_deletion_info&, form_stub&);
```

The form can decide whether to sever a given use based on the parameters given to the deletion request. For example, if an idle is being flagged as deleted, and it knows that its own deletion is happening because an ancestor is being flagged as deleted, then it can decide *not* to sever its use of that ancestor. This means that if you delete a tree of idles and that tree is only flagged as deleted, you don't end up with a bunch of individual loose idles that are flagged as deleted; instead, the topmost idle in that tree is made loose, and the also-flagged-as-deleted descendants *remain* descendants.

The function would return `true` if all uses of the given form are severed, and `false` if any uses aren't severed. If the given form is to be wholly deleted, not merely flagged, then in the core deletion code, we `assert` that this function returned `true`.

