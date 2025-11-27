
class Datastore {
   #is_building = false;
   
   constructor() {
      this.action_editor_ids = []; // Array<String>
      this.idles_by_id = new Map(); // Map<String editor_id, Idle>
      this.graphs      = []; // Array<Graph>
      this.loose       = new LooseIdleList(null);
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
      for(let anam_and_dnam of idle_node.form.serialized.masters) {
         let action = this.#ensure_action_for_building(anam_and_dnam);
         if (!action)
            continue;
         action._track_candidate(anam_and_dnam, idle_node, false);
         idle_node._track_candidacy(action, false);
      }
      for(let anam_and_dnam of idle_node.form.serialized.active) {
         let action = this.#ensure_action_for_building(anam_and_dnam);
         if (!action)
            continue;
         action._track_candidate(anam_and_dnam, idle_node, true);
         idle_node._track_candidacy(action, true);
      }
   }
   
   #place_forced_loose_idle(/*Idle*/ idle_node) {
      let graph = this.graph_by_idle(idle_node);
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
   
};