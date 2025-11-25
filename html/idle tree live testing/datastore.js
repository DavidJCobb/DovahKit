
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
            let _clone_serialized = function(src_list) {
               let dst_list = [];
               for(let src of src_list)
                  dst_list.push(src.clone());
               return dst_list;
            };
            
            let node = new Idle(form.editor_id);
            this.idles_by_id.set(form.editor_id, node);
            node.serialized.masters = _clone_serialized(form.serialized.masters);
            node.serialized.active  = _clone_serialized(form.serialized.active);
         }
         //
         // Build the parent/child hierarchy for the idle nodes.
         //
         for(let idle_node of this.idle_forms.values()) {
            this.#place_action_root(idle_node);
            if (idle_node.flags.is_parent) {
               this.#place_parent_idle(idle_node);
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
         for(let idle_node of this.idle_forms.values()) {
            this.#post_placement_parentage_validation(idle_node);
         }
      } finally {
         this.#is_building = false;
      }
   }

   /*Graph*/ graph_by_idle(/*Idle*/ idle) {
      const path = idle.canonical_graph_path;
      for(let graph of this.graphs)
         if (graph.path == path)
            return graph;
      return null;
   }
   /*Graph*/ #get_or_create_graph(/*String*/ path) {
      // TODO
      throw new Error("NOT YET IMPLEMENTED");
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
      for(let anam_and_dnam of idle_node.serialized.masters) {
         let action = this.#ensure_action_for_building(anam_and_dnam);
         if (!action)
            continue;
         action._track_candidate(anam_and_dnam, idle_node, false);
      }
      for(let anam_and_dnam of idle_node.serialized.active) {
         let action = this.#ensure_action_for_building(anam_and_dnam);
         if (!action)
            continue;
         action._track_candidate(anam_and_dnam, idle_node, true);
      }
   }
   
   #place_parent_idle(/*Idle*/ idle_node) {
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
   }
   
   #idle_has_cyclical_parentage(/*Set<IdleForm>*/ seen, /*IdleNode*/ idle) {
      // TODO
      throw new Error("NOT YET IMPLEMENTED");
   }
   
   #place_child_idle(/*Idle*/ idle_node) {
      let canonical_graph_path = idle_node.canonical_graph_path;
      
      let parent_idle   = null; // Optional<IdleForm>
      let previous_idle = null; // Optional<IdleForm>
      let is_action_root_in_own_graph = false;
      {
         const active  = idle_node.serialized.active;
         const masters = idle_node.serialized.masters;
         
         const list = active.length ? active : masters;
         if (list.length > 0) {
            let item = list[list.length - 1];
            parent_idle   = item.parent;
            previous_idle = item.previous;
            if (!(parent_idle instanceof IdleForm))
               parent_idle = null;
         }
         for(let item of idle_node.serialized.masters) {
            if (item.parent instanceof ActionForm && item.graph == canonical_graph_path) {
               is_action_root_in_own_graph = true;
               break;
            }
         }
         if (!action) {
            for(let item of idle_node.serialized.active) {
               if (item.parent instanceof ActionForm && item.graph == canonical_graph_path) {
                  is_action_root_in_own_graph = true;
                  break;
               }
            }
         }
      }
      
      let loose_parent_node = null; // Optional<LooseIdleList>
      
      let seen_ancestors = new Set(); // Set<IdleForm>
      
      // Handle cyclical parents
      if (this.#idle_has_cyclical_parentage(seen_ancestors, idle_node)) {
         console.warn("Cyclical parent relationships: ", idle_node);
         parent_idle   = null;
         previous_idle = null;
      } else {
         // TODO: Handle invalid preivous-siblings
         throw new Error("NOT YET COMPLETE");
      }
      
      let parent_node   = null; // Optional<Idle>
      let previous_node = null; // Optional<Idle>
      // TODO: Find parent-node and previous-node given parent-idle-form and 
      // previous-idle-form.
      throw new Error("NOT YET COMPLETE");
      
      // Update "sort state" on idles to reflect any severed parent or 
      // previous-sibling relationships above. This will be used when we 
      // perform sorted insertions into a parent idle.
      idle_node._sort_state.parent   = parent_node;
      idle_node._sort_state.previous = previous_node;
      
      if (parent_node) {
         parent_node.insert_sorted_child(idle_node);
         return;
      }
      
      if (is_action_root_in_own_graph) {
         //
         // The idle can't be loose if it's an action root in its containing 
         // graph.
         //
         return;
      }
      if (!loose_parent_node) {
         console.warn("Orphaned idle: ", idle_node);
         loose_parent_node = this.loose;
      }
      loose_parent_node.idles.push(idle_node);
   }
   
   #post_placement_parentage_validation(/*Idle*/ idle_node) {
      // TODO
      throw new Error("NOT YET IMPLEMENTED");
   }
   
};