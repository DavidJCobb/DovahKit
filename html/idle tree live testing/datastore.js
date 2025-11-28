
class Datastore {
   #is_building = false;
   
   constructor() {
      this.idles_by_id = new Map(); // Map<String editor_id, Idle>
      this.graphs      = []; // Array<Graph>
      this.loose       = new LooseIdleList(null);
      
      // These callbacks exist to accommodate QAbstractItemModel, which 
      // generally needs to do certain bookkeeping tasks before and after 
      // operations such as the movement, insertion, or deletion of model 
      // nodes.
      this.callbacks = {
         node_moved: {
            before: null,
            after:  null,
         },
         node_deleted: {
            before: null,
            after:  null,
         },
         
         // These callbacks don't need "before" or "after" variants. 
         // They exist for when an idle becomes, or ceases to be, present 
         // in multiple places at once, and are fired only for the idle's 
         // non-canonical parents.
         //
         // In general, the reason QAbstractItemModel needs "before" 
         // and "after" behaviors is so it can prepare to adjust its 
         // QPersistentModelIndexes in response to the insertion or 
         // removal of nodes: if you insert or remove a node, you will 
         // displace its siblings; additionally, if you move a node, then 
         // that node's own QPMI also needs adjustment; and if you remove 
         // a node, then that node's QPMI needs to be invalidated.
         //
         // However, if an idle is present in multiple places at once, 
         // all but one of those places *must* be an action, and the last 
         // one (the canonical parent) *may* be an action node. We fire 
         // these callbacks for non-canonical parents (i.e. the places 
         // that *must* be actions). In those cases, there can't be any 
         // siblings to adjust: these callbacks represent the insertion 
         // or removal of an only-child.
         //
         // The appropriate response by a QAbstractModelIndex, then, is 
         // to call beginInsertRows and endInsertRows immediately in 
         // response to the former callback, and beginRemoveRows and 
         // endRemoveRows immediately in response to the latter callback. 
         // The parent should be the QMI of the action, the row index 
         // should always be 0, and the number of rows being inserted or 
         // removed should always be 1.
         idle_becoming_multiply_present:  null,
         idle_no_longer_multiply_present: null,
      };
   }
   
   get is_building() { return this.#is_building; }
   
   build(
      /*Array<ActionForm>*/ action_forms,
      /*Array<IdleForm>*/   idle_forms
   ) {
      this.#is_building = true;
      try {
         //
         // Pre-create all idle nodes.
         //
         for(let form of idle_forms) {
            let node = new Idle(form.editor_id, form);
            this.idles_by_id.set(form.editor_id, node);
            
            this.#place_action_root(node);
         }
         //
         // Build the parent/child hierarchy for the idle nodes.
         //
         for(let idle_node of this.idles_by_id.values()) {
            if (idle_node.form.flags.is_forced_loose) {
               this.#place_forced_loose_idle(idle_node);
            } else {
               this.#place_child_idle(idle_node);
            }
         }
         //
         this.graphs.sort(function(a, b) {
            return a.path.localeCompare(b.path);
         });
         for(let graph of this.graphs) {
            graph.actions.sort(function(a, b) {
               return a.editor_id.localeCompare(b.editor_id);
            });
         }
         //
         for(let idle_node of this.idles_by_id.values()) {
            this.#post_placement_parentage_validation(idle_node);
         }
      } finally {
         this.#is_building = false;
      }
   }

   /*Graph*/ graph_by_path(/*String*/ path) {
      for(let graph of this.graphs)
         if (graph.path == path)
            return graph;
      return null;
   }
   /*Graph*/ graph_by_idle(/*const Idle*/ idle) {
      return this.graph_by_path(idle.canonical_graph_path);
   }
   /*Graph*/ #get_or_create_graph(/*String*/ path) {
      for(let graph of this.graphs)
         if (graph.path == path)
            return graph;
      let graph = new Graph(path);
      this.graphs.push(graph);
      return graph;
   }
   
   #ensure_action_for_building(/*IdleSerialized*/ anam_and_dnam) {
      let form = anam_and_dnam.parent;
      if (!form || !(form instanceof ActionForm))
         return null;
      let graph = this.#get_or_create_graph(anam_and_dnam.graph);
      if (!graph)
         return null;
      return graph.get_or_create_action(form.editor_id);
   }
   
   #place_action_root(/*Idle*/ idle_node) {
      let first_action   = null;
      let actions_differ = false;
      
      let _on_action = function(action) {
         if (actions_differ)
            return;
         if (first_action) {
            actions_differ = action != first_action;
         } else {
            first_action = action;
         }
      };
      
      for(let anam_and_dnam of idle_node.form.serialized.masters) {
         let action = this.#ensure_action_for_building(anam_and_dnam);
         if (!action)
            continue;
         _on_action(action);
         action._track_candidate(anam_and_dnam, idle_node, true);
         idle_node._track_candidacy(action, true);
      }
      for(let anam_and_dnam of idle_node.form.serialized.active) {
         let action = this.#ensure_action_for_building(anam_and_dnam);
         if (!action)
            continue;
         _on_action(action);
         action._track_candidate(anam_and_dnam, idle_node, false);
         idle_node._track_candidacy(action, false);
      }
      
      if (actions_differ) {
         console.warn("Idle attempts to be the root of multiple actions. This can happen if an override attempts to re-parent an action root, and can result in the idle ending up in multiple places at once. More rarely, it could happen if a malformed IDLE record contains multiple ANAM subrecords placing the same idle as different action roots. ", idle_node);
      }
   }
   
   #place_forced_loose_idle(/*Idle*/ idle_node) {
      let graph = this.#get_or_create_graph(idle_node.canonical_graph_path);
      let loose;
      if (graph)
         loose = graph.loose;
      else {
         console.warn("Error: No loose idle list for idle", idle_node);
         loose  = this.loose;
      }
      console.assert(loose instanceof LooseIdleList);
      loose.idles.push(idle_node);
      idle_node.live.parent = loose;
      
      if (idle_node.candidacies.masters.length || idle_node.candidacies.active.length) {
         console.warn("Idle is an action root, but is also flagged as loose, and so risks ending up in multiple places at once: ", idle_node);
      }
      if (idle_node.form.hierarchy_parent && !(idle_node.form.hierarchy_parent instanceof ActionForm)) {
         console.warn("Idle is set to be the child of another idle, but is also flagged as loose, so it will not in fact be a child. Is this intentional? ", idle_node);
      }
   }
   
   #idle_has_cyclical_parentage(/*Set<IdleForm>*/ seen, /*IdleNode*/ node) {
      let form    = node.form;
      let current = form.hierarchy_parent;
      do {
         if (!current)
            break;
         if (!(current instanceof IdleForm))
            break;
         if (seen.has(current))
            return true;
         seen.add(current);
         
         current = current.hierarchy_parent;
      } while (true);
      return false;
   }
   
   #idle_has_bad_siblinghood(/*Set<IdleForm>*/ seen_ancestors, /*IdleNode*/ node) {
      let form   = node.form;
      let parent = form.hierarchy_parent;
      if (!(parent instanceof IdleForm))
         parent = null;
      
      let current = form.hierarchy_previous;
      if (!(current instanceof IdleForm))
         return false;
      
      let seen_siblings = new Set(); // Set<IdleForm>
      do {
         if (seen_ancestors.has(current))
            return "ancestor";
         if (seen_siblings.has(current))
            return "cyclical";
         seen_siblings.add(current);
         
         let current_parent = current.hierarchy_parent;
         if (!(current_parent instanceof IdleForm))
            current_parent = null;
         if (current_parent != parent)
            return "mismatched";
         
         current = current.hierarchy_previous;
         if (!(current instanceof IdleForm))
            break;
      } while (true);
      return false;
   }
   
   #place_child_idle(/*Idle*/ idle_node) {
      const canonical_graph_path = idle_node.canonical_graph_path;
      
      let parent_idle   = idle_node.form.hierarchy_parent;   // Optional<IdleForm>
      let previous_idle = idle_node.form.hierarchy_previous; // Optional<IdleForm>
      const is_action_root_in_own_graph = idle_node.is_winning_root_of_action_in_own_graph();
      
      let loose_parent_node = null; // Optional<LooseIdleList>
      
      let seen_ancestors = new Set(); // Set<IdleForm>
      
      // Handle cyclical parents
      if (this.#idle_has_cyclical_parentage(seen_ancestors, idle_node)) {
         console.warn("Cyclical parent relationships: ", idle_node);
         parent_idle   = null;
         previous_idle = null;
      } else {
         // Handle invalid previous siblings
         let problem = this.#idle_has_bad_siblinghood(seen_ancestors, idle_node);
         if (problem) {
            console.warn("Idle has invalid sibling: ", idle_node, problem);
            {
               let graph = this.graph_by_path(canonical_graph_path);
               if (!graph && parent_idle) {
                  let path = parent_idle.canonical_graph_path;
                  if (path)
                     graph = this.graph_by_path(path);
               }
               if (!graph && previous_idle) {
                  let path = previous_idle.canonical_graph_path;
                  if (path)
                     graph = this.graph_by_path(path);
               }
               if (graph)
                  loose_parent_node = graph.loose;
            }
            parent_idle   = null;
            previous_idle = null;
         }
      }
      //
      // A minor note about the corrections made to the idle pointers 
      // above. These are the same corrections that the game makes, but 
      // this case, we don't *retain* the corrections. That is: the game 
      // directly modifies the loaded `TESIdleForm` objects in memory to 
      // have the corrected pointers, whereas we don't.
      //
      // The only effect this should have is that when we build the trees, 
      // we can't early-out as quickly. If for example there's a cyclical 
      // sibling relationship between A < B < C < D < A, then:
      //
      //  - The game nulls out A's previous-sibling and parent; then when 
      //    it checks B, B has a different parent from A and fails right 
      //    off rip. This cascades such that C and D both fail quickly as 
      //    well.
      //
      //  - ...whereas DovahKit has to detect the full cycle every time 
      //    it processes any of the siblings in that cycle.
      //
      // We still produce the same tree as the CK once we're done, and 
      // doing it this way is simpler. As a potential optimization, we 
      // could split building from two steps into three: create all idle 
      // nodes; then [new step] set all idle nodes' sort states blindly 
      // based on what their ANAM wants; then, in this function, we'd use 
      // (and modify) the sort states alone, allowing us to early-out 
      // exactly as the CK does. But having that extra step would be slower 
      // to no benefit when dealing with well-formed idle trees.
      //
      
      // Find parent-node and previous-node given parent-idle-form and 
      // previous-idle-form.
      let parent_node   = null; // Optional<Idle>
      let previous_node = null; // Optional<Idle>
      if (parent_idle)
         parent_node = this.idles_by_id.get(parent_idle.editor_id);
      if (previous_idle)
         previous_node = this.idles_by_id.get(previous_idle.editor_id);
      
      // Update "sort state" on idles to reflect any severed parent or 
      // previous-sibling relationships above. This will be used when we 
      // perform sorted insertions into a parent idle.
      idle_node._sort_state.parent   = parent_node;
      idle_node._sort_state.previous = previous_node;
      
      if (parent_node) {
         if (idle_node.candidacies.masters.length || idle_node.candidacies.active.length) {
            console.warn("Idle has been placed as both an action root and a child idle, and so may end up in multiple places at once. ", idle_node);
         }
         parent_node._insert_sorted_child(idle_node);
         return;
      }
      
      if (is_action_root_in_own_graph) {
         //
         // The idle can't be loose if it's an action root in its containing 
         // graph.
         //
         return;
      }
      if (loose_parent_node) {
         // Same warning as the CK, but with more precise wording.
         console.warn("Idle has ended up loose, but wasn't originally flagged as loose. Is this intentional? ", idle_node);
      } else {
         if (canonical_graph_path) {
            loose_parent_node = this.#get_or_create_graph(canonical_graph_path).loose;
            
            // Same warning as the CK, but with more precise wording.
            console.warn("Idle has ended up loose, but wasn't originally flagged as loose. Is this intentional? ", idle_node);
         } else {
            loose_parent_node = this.loose;
            
            console.warn("Orphaned idle: ", idle_node);
         }
      }
      loose_parent_node.idles.push(idle_node);
      idle_node.live.parent = loose_parent_node;
   }
   
   #post_placement_parentage_validation(/*Idle*/ idle_node) {
      let previous = idle_node._sort_state.previous;
      let parent   = idle_node._sort_state.parent;
      idle_node._sort_state = { parent: null, previous: null };
      
      if (parent) { // if idle_node.live.parent is an action, parent should be null
         console.assert(parent == idle_node.live.parent);
      } else {
         console.assert(!(idle_node.live.parent instanceof Idle));
      }
      
      if (!parent)
         return;
      const siblings = parent.live.children;
      
      let i = siblings.indexOf(idle_node);
      if (i < 0) {
         console.warn("Inconsistent parentage on ", idle_node);
      }
      if (previous) {
         let actual = null;
         if (i > 0)
            actual = siblings[i - 1];
         if (i == 0 || actual != previous)
            console.warn("Previous-sibling on idle is not what it wanted: ", idle_node);
      }
   }
   
   //
   // POST-BUILD DATASTORE OPERATIONS
   //
   
   // This only checks whether a given movement would produce a result which is 
   // representable given the file format and tree-building algorithm. This is 
   // not intended to prevent moves that are merely bad ideas (e.g. moves that 
   // would cause the tree to be degenerate in a way that: Bethesda doesn't 
   // guard against, and that therefore has to be representable by our code). 
   // For those, see `is_idle_movement_a_really_bad_idea`.
   /*bool*/ is_idle_movement_legal(/*const Idle*/ subject, /*const Variant<Idle, LooseIdleList, Action>*/ dst_parent, /*const Optional<Idle>*/ dst_previous) /*const*/ {
      // Moving an idle into itself or its descendants is illegal.
      if (dst_parent instanceof Idle)
         if (subject == dst_parent || subject.contains(dst_parent))
            return false;
      
      return true;
   }
   
   /*bool*/ is_idle_movement_a_really_bad_idea(/*const Idle*/ subject, /*const Variant<Idle, LooseIdleList, Action>*/ dst_parent, /*const Optional<Idle>*/ dst_previous) /*const*/ {
      // Moving action roots is a bad idea.
      if (subject.live.parent instanceof Action)
         return true;
      
      // Displacing action roots is a bad idea.
      if (dst_parent instanceof Action)
         if (dst_parent.root)
            return true;
      
      // Bethesda doesn't intend for loose idles to have children, so moving 
      // an idle that has children into LOOSE is a bad idea.
      if (subject.children.length)
         if (dst_parent instanceof LooseIdleList)
            return true;
      
      return false;
   }
   
   /*bool*/ is_idle_deletion_legal(/*const Idle*/ subject) {
      return true;
   }
   
   /*bool*/ is_idle_deletion_a_really_bad_idea(/*const Idle*/ subject) {
      // Deleting an action-root defined by a non-active file is a bad idea, 
      // since it won't necessarily stop being the root for that action.
      if (subject.candidacies.master.length > 0)
         return true;
      
      return false;
   }
   
   #on_runner_up_becoming_root(/*Action*/ action, /*Idle*/ idle) {
      if (idle.live.parent == action) {
         return;
      }
      if (idle.live.parent == action.graph.loose) {
         let loose = action.graph.loose;
         if (idle.form.flags.is_forced_loose) {
            action.root = idle;
            if (this.callbacks.idle_becoming_multiply_present)
               (this.callbacks.idle_becoming_multiply_present)(idle, action);
         } else {
            if (this.callbacks.node_moved.before)
               (this.callbacks.node_moved.before)(idle, action, 0);
            loose.idles.splice(loose.idles.indexOf(idle), 1);
            action.root = idle;
            idle.live.parent = action;
            if (this.callbacks.node_moved.after)
               (this.callbacks.node_moved.after)(idle);
         }
      } else {
         action.root = idle;
         if (this.callbacks.idle_becoming_multiply_present)
            (this.callbacks.idle_becoming_multiply_present)(idle, action);
      }
   }
   
   // This should be invoked for an idle before it is moved or deleted. If the 
   // idle is an active-file candidate for any action roots besides its canonical 
   // parent, then [by definition the idle is in multiple places, and] this severs 
   // those candidacies and emits callbacks for the idle no longer being multiply 
   // present in those locations.
   //
   // The canonical parent is skipped here, and should be handled by the caller 
   // as appropriate for the given operation (move versus delete).
   #destroy_non_canonical_active_root_candidacies(/*Idle*/ subject) {
      const list = subject.candidacies.active;
      let   size = list.length;
      for(let i = 0; i < size; ++i) {
         const action = list[i];
         if (action == subject.live.parent) // if canonical parent is an action, skip it
            continue;
         const was_root = action.root == subject;
         action._untrack_active_file_candidate(subject);
         if (was_root) {
            action._recalc_winning_root();
            if (action.root != subject) {
               if (this.callbacks.idle_no_longer_multiply_present)
                  this.callbacks.idle_no_longer_multiply_present(subject, action);
            }
            if (action.root) {
               this.#on_runner_up_becoming_root(action, action.root);
            }
         }
         list.splice(i, 1);
         --i;
         --size;
      }
   }
   
   // To be invoked as part of move- or delete-idle operations. Returns the 
   // idle's former next-sibling. The caller must have already severed the 
   // idle's active-file action root candidacies, and is responsible for 
   // telling the idle to where it has been relocated.
   /*Idle*/ #take_idle_from_canonical_parent(taken_from, /*Idle*/ idle) {
      let former_next_sibling = null;
      if (taken_from instanceof Action) {
         taken_from._untrack_active_file_candidate(idle);
         console.assert(idle.candidacies.active.length <= 1);
         idle.candidacies.active = [];
      } else {
         console.assert(idle.candidacies.active.length == 0);
         if (taken_from instanceof Idle) {
            let i = taken_from.live.children.indexOf(idle) + 1;
            if (i < taken_from.live.children.length)
               former_next_sibling = taken_from.live.children[i];
            taken_from.live.children.splice(i - 1, 1);
         } else if (taken_from instanceof LooseIdleList) {
            let i = taken_from.idles.indexOf(idle);
            taken_from.idles.splice(i, 1);
         }
      }
      return former_next_sibling;
   }
   
   /*void*/ #update_canonical_parent_action_after_root_taken(/*Action*/ taken_from, /*Idle*/ taken_idle) {
      taken_from._recalc_winning_root();
      if (taken_idle == taken_from.root) {
         //
         // If `taken_idle` is still the winning root of the action we just 
         // moved it from, then it must now be present in multiple places, with 
         // its canonical parent being somewhere else.
         //
         if (this.callbacks.idle_becoming_multiply_present)
            (this.callbacks.idle_becoming_multiply_present)(taken_idle, taken_from);
      } else if (taken_from.root) {
         this.#on_runner_up_becoming_root(taken_from, taken_from.root);
      }
   }
   
   // Use only for fully deleting an idle out of existence. For merely flagging 
   // an IDLE's override record as "deleted," do not invoke this. When we save 
   // an IDLE record, we always include DNAM and ANAM, so the "deleted" flag 
   // allowing those to bleed through from the previous record is irrelevant 
   // because the blood gets wiped off the floor, walls, and ceiling anyway.
   /*void*/ delete_idle(/*Idle*/ subject) {
      let defined_in_master = subject.form.serialized.masters.length > 0;
      
      if (defined_in_master) {
         //
         // The IDLE form was originally defined in a master file and so cannot 
         // be wholly deleted. The most we can do is move it to LOOSE and flag 
         // it and its descendants as "deleted."
         //
         ; // flag the idle form as "deleted" here.
         //
         let graph = null;
         {
            let parent = subject.live.parent;
            while (parent) {
               if (parent instanceof Action || parent instanceof LooseIdleList) {
                  graph = parent.graph;
                  break;
               }
               if (parent instanceof Idle) {
                  parent = parent.live.parent;
                  continue;
               }
               console.assert(false, "unhandled case!");
            }
         }
         let loose = null;
         if (graph)
            loose = graph.loose;
         else
            loose = this.loose;
         this.move_idle(subject, loose, null);
      }
      
      // Recursively delete descendant idles.
      {
         let children = ([]).concat(subject.live.children); // copy array
         for(let child of children) {
            this.delete_idle(child);
         }
      }
      
      if (defined_in_master) {
         return;
      }
      
      // Destroy the subject's active-file action root candidacies, except that 
      // pertaining to its canonical parent. (That particular candidacy will be 
      // destroyed when we delete the subject, further below.)
      this.#destroy_non_canonical_active_root_candidacies(subject);
      
      let form = subject.form;
      
      if (this.callbacks.node_deleted.before)
         (this.callbacks.node_deleted.before)(subject);
      
      const moved_from          = subject.live.parent;
      let   former_next_sibling = this.#take_idle_from_canonical_parent(moved_from, subject);
      subject.live.parent = null;
      if (former_next_sibling)
         former_next_sibling._update_form_hierarchy_data();
      this.idles_by_id.delete(form.editor_id);
      subject._update_form_hierarchy_data();
      subject.form.editor_id = "DELETED DELETED DELETED";
      
      // Explicit `delete` instructions don't actually delete the pointed-to 
      // object; they just reset the identifier (or cause browsers to whine 
      // at you and spew syntax errors in strict mode, I guess). This is just 
      // here to indicate where and when we'd delete the node in C++.
      //delete subject;
      
      if (this.callbacks.node_deleted.after)
         (this.callbacks.node_deleted.after)(form);
      
      // Update the action the subject was deleted from (if any).
      if (moved_from instanceof Action) {
         this.#update_canonical_parent_action_after_root_taken(moved_from, subject);
      }
   }
   
   /*void*/ move_idle(/*Idle*/ subject, /*Variant<Idle, LooseIdleList, Action>*/ dst_parent, /*Optional<Idle>*/ dst_previous) {
      // Skip redundant operations.
      if (subject.live.parent == dst_parent) {
         if (dst_parent instanceof Idle) {
            if (dst_previous) {
               let i = dst_parent.live.children.indexOf(subject);
               console.assert(i >= 0);
               if (i > 0 && dst_parent.live.children[i - 1] == dst_previous)
                  return;
            } else {
               if (dst_parent.live.children[0] == subject)
                  return;
            }
            //
            // Else movement proceeds, because the idle can still be reordered 
            // within its parent. (All other parent types are unordered.)
            //
         } else if (dst_parent instanceof LooseIdleList) {
            if (!subject.form.flags.is_forced_loose) {
               //
               // If a subject happened to *end up* in LOOSE, rather than being 
               // made loose intentionally, then an explicit move to LOOSE is 
               // not strictly a no-op; make the subject intentionally loose and 
               // then exit.
               //
               subject._update_form_hierarchy_data();
               return;
            }
         } else {
            return;
         }
      }
      
      // Skip impossible operations.
      if (!this.is_idle_movement_legal(subject, dst_parent, dst_previous))
         return;
      
      // If moving to an action, displace any action root which is already there.
      if (dst_parent instanceof Action) {
         if (dst_parent.root) {
            let graph     = dst_parent.graph;
            let displaced = dst_parent.root;
            if (displaced) {
               dst_parent.root = null;
               if (displaced.live.parent == dst_parent) {
                  //
                  // The destination is the to-be-displaced idle's canonical parent, 
                  // so that idle must be moved.
                  //
                  if (displaced.is_active_winning_root_of(dst_parent)) {
                     //
                     // The to-be-displaced idle is placed here by the active file, 
                     // so let's do a fully-fledged move operation to make it a 
                     // loose idle within the active file.
                     //
                     console.assert(!!graph);
                     this.move_idle(displaced, graph.loose, null);
                  } else {
                     //
                     // The to-be-displaced idle is placed here by a master file, 
                     // so it'll be displaced to a loose idle. The difference 
                     // between this and the contrary branch is the difference 
                     // between the idle being "made" a loose idle versus it 
                     // "ending up as" a loose idle.
                     //
                     let i = upper_bound(
                        graph.loose.idles,
                        displaced,
                        function(a, b) {
                           return a.form.editor_id.localeCompare(b.form.editor_id);
                        }
                     );
                     
                     if (this.callbacks.node_moved.before)
                        (this.callbacks.node_moved.before)(displaced, graph.loose, i);
                     graph.loose.idles.splice(i, 0, displaced);
                     displaced.live.parent = graph.loose;
                     if (this.callbacks.node_moved.after)
                        (this.callbacks.node_moved.after)(displaced);
                  }
               } else {
                  //
                  // The to-be-displaced idle is in multiple places at once, the 
                  // destination is one of those, and the destination is not the 
                  // to-be-displaced idle's canonical parent. So, that idle ceases 
                  // to be at the destination, but it does not "move" per se.
                  //
                  if (this.callbacks.idle_no_longer_multiply_present)
                     this.callbacks.idle_no_longer_multiply_present(displaced, dst_parent);
               }
            }
         }
      }
      
      // Destroy the subject's active-file action root candidacies, except that 
      // pertaining to its canonical parent. (That particular candidacy will be 
      // destroyed when we move the subject, further below.)
      this.#destroy_non_canonical_active_root_candidacies(subject);
      
      let insert_at = 0;
      if (dst_parent instanceof Idle) {
         if (dst_previous) {
            insert_at = dst_parent.live.children.indexOf(dst_previous) + 1;
         }
      } else if (dst_parent instanceof LooseIdleList) {
         insert_at = upper_bound(
            dst_parent.idles,
            subject,
            function(a, b) {
               return a.form.editor_id.localeCompare(b.form.editor_id);
            }
         );
      }
      
      if (this.callbacks.node_moved.before)
         (this.callbacks.node_moved.before)(subject, dst_parent, insert_at);
      
      const moved_from = subject.live.parent;
      if (moved_from == dst_parent) {
         let moved_from_index = insert_at + 1;
         if (moved_from instanceof Idle) {
            moved_from_index = moved_from.live.children.indexOf(subject);
         } else if (moved_from instanceof LooseIdleList) {
            moved_from_index = moved_from.idles.indexOf(subject);
         }
         if (insert_at >= moved_from_index) {
            --insert_at;
         }
      }
      let former_next_sibling = this.#take_idle_from_canonical_parent(moved_from, subject);
      subject.live.parent = dst_parent;
      if (dst_parent instanceof Action) {
         subject._track_candidacy(dst_parent, false);
         dst_parent.candidacies.active.push({
            idle: subject,
            info: null,
         });
         dst_parent.root = subject;
      } else if (dst_parent instanceof LooseIdleList) {
         dst_parent.idles.splice(insert_at, 0, subject);
      } else if (dst_parent instanceof Idle) {
         dst_parent.live.children.splice(insert_at, 0, subject);
         let new_next_sibling = dst_parent.live.children[insert_at + 1];
         if (new_next_sibling)
            new_next_sibling._update_form_hierarchy_data();
      } else {
         console.assert(false);
      }
      let graph_path_prior = subject.canonical_graph_path;
      subject._update_form_hierarchy_data();
      let graph_path_after = subject.canonical_graph_path;
      if (former_next_sibling)
         former_next_sibling._update_form_hierarchy_data();
      
      if (this.callbacks.node_moved.after)
         (this.callbacks.node_moved.after)(subject);
      
      // Update the action the subject was moved from (if any).
      if (moved_from instanceof Action) {
         this.#update_canonical_parent_action_after_root_taken(moved_from, subject);
      }
      
      // If the subject was moved across graphs, update form data for all of its 
      // descendants.
      if (graph_path_prior != graph_path_after) {
         //
         // Moving the subject across graphs should, in general, force changes 
         // to all of its descendants to update their DNAM subrecords. I don't 
         // believe that's strictly necessary, but it seems like it'd make for 
         // the cleanest serialized data.
         //
         (function _recurse(parent) {
            for(let idle of parent.live.children) {
               idle._update_form_hierarchy_data();
               _recurse(idle);
            }
         })(subject);
      }
   }
   
};