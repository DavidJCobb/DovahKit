
class Graph {
   constructor(/*String*/ path) {
      this.path    = path; // IDLE/DNAM
      this.actions = []; // Array<Action>
      this.loose   = new LooseIdleList(this);
   }
   
   /*Action*/ get_or_create_action(/*String*/ editor_id) {
      for(let action of this.actions)
         if (action.editor_id == editor_id)
            return action;
      let action = new Action(editor_id);
      this.actions.push(action);
      return action;
   }
};

class LooseIdleList {
   constructor(/*Optional<Graph>*/ graph) {
      this.graph = graph; // Graph
      this.idles = []; // Arrtay<Idle>
   }
};

class ActionForm {
   constructor(/*String*/ editor_id) {
      this.editor_id = editor_id;
   }
   
   /*Object*/ as_serialized() /*const*/ {
      return {
         editor_id: this.editor_id,
      };
   }
};

class Action {
   constructor(/*String*/ editor_id) {
      this.graph     = null; // Graph
      this.editor_id = editor_id;
      this.root      = null; // Optional<Idle>
      this.candidacies = {
         masters: [],
         active:  []
      };
   }
   
   // for initial build only
   _track_candidate(/*IdleSerialized*/ candidacy, /*Idle*/ idle, /*bool*/ via_master) {
      let list = via_master ? this.candidacies.masters : this.candidacies.active;
      list.push({
         idle: idle,
         info: candidacy,
      });
      if (!this.root)
         this.root = idle;
   }
};

class IdleSerialized {
   constructor() {
      this.graph    = "";   // IDLE/DNAM
      this.parent   = null; // IDLE/ANAM: ActionForm or IdleForm
      this.previous = null; // IDLE/ANAM: IdleForm
   }
   /*IdleSerialized*/ clone() /*const*/ {
      let copy = new IdleSerialized();
      copy.graph    = this.graph;
      copy.parent   = this.parent;
      copy.previous = this.previous;
      if (copy.parent instanceof Idle)
         copy.parent = copy.parent.editor_id;
      if (copy.previous instanceof Idle)
         copy.previous = copy.previous.editor_id;
      return copy;
   }
};

class IdleForm {
   constructor(options) {
      this.editor_id = options.editor_id;  // String
      this.flags = {
         is_parent: false,
      };
      this.serialized = {
         masters: options.serialized?.masters || [], // Array<IdleSerialized>
         active:  options.serialized?.active  || [], // Array<IdleSerialized>
      };
   }
   
   get hierarchy_parent() {
      let list = this.serialized.active;
      if (!list.length) {
         list = this.serialized.masters;
         if (!list.length)
            return null;
      }
      return list[list.length - 1].parent;
   }
   get hierarchy_previous() {
      let list = this.serialized.active;
      if (!list.length) {
         list = this.serialized.masters;
         if (!list.length)
            return null;
      }
      return list[list.length - 1].previous;
   }
   
   /*String*/ get canonical_graph_path() {
      let list = this.serialized.active;
      if (!list.length) {
         list = this.serialized.masters;
         if (!list.length)
            return "";
      }
      return list[list.length - 1].graph;
   }
   
   /*Object*/ as_serialized() /*const*/ {
      let out = {
         editor_id: this.editor_id,
         subrecords: {
            masters: [],
            active:  [],
         },
      };
      for(let item of this.serialized.masters) {
         out.subrecords.masters.push({
            signature: "DNAM",
            string:    item.graph
         });
         out.subrecords.masters.push({
            signature: "ANAM",
            parent:    item.parent   ? item.parent.editor_id   : null,
            previous:  item.previous ? item.previous.editor_id : null,
         });
      }
      if (this.flags.is_parent) {
         out.subrecords.active.push({
            signature: "DATA",
            is_parent: true,
         });
      }
      for(let item of this.serialized.active) {
         out.subrecords.active.push({
            signature: "DNAM",
            string:    item.graph
         });
         out.subrecords.active.push({
            signature: "ANAM",
            parent:    item.parent   ? item.parent.editor_id   : null,
            previous:  item.previous ? item.previous.editor_id : null,
         });
      }
      return out;
   }
};

class Idle {
   constructor(/*String*/ editor_id, /*IdleForm*/ form) {
      this.editor_id  = editor_id;
      this.form  = form;
      this.flags = {
         is_parent: false,
      };
      this.serialized = {
         masters: [], // Array<IdleSerialized>
         active:  [], // Array<IdleSerialized>
      };
      this._sort_state = { // used during initial build
         parent:   null, // Idle
         previous: null, // Idle
      };
      this.live = {
         parent:   null, // Variant<null, LooseIdleList, Action, Idle>
         children: [],   // Array<Idle>
      };
   }
   
   /*String*/ get canonical_graph_path() {
      return this.form.canonical_graph_path;
   }
   
   _insert_sorted_child(/*Idle*/ idle) {
      const children = this.live.children;
      {
         let desired_prev = idle._sort_state.previous;
         if (desired_prev) {
            let i = children.indexOf(desired_prev);
            if (i >= 0)
               children.splice(i + 1, 0, idle);
            else
               children.push(idle);
         } else {
            children.push(idle);
         }
         idle.live.parent = this;
      }
      
      let current_node = idle;
      let next_node    = null;
      do {
         let current_index = -1;
         let next_index    = -1;
         for(let i = 0; i < children.length; ++i) {
            let candidate = children[i];
            if (candidate == current_node) {
               current_index = i;
               continue;
            }
            if (candidate._sort_state.previous == current_node) {
               if (next_node) {
                  console.warn("Multiple next-siblings for idle ", idle);
               } else {
                  next_node  = candidate;
                  next_index = i;
               }
            }
         }
         if (next_index == -1)
            break;
         if (next_index == current_index + 1)
            break;
         console.assert(!!next_node);
         
         // move-item-after-index
         let at = current_index + 1;
         if (at > next_index)
            --at;
         children.splice(next_index, 1);
         children.splice(at, 0, next_node);
         
         current_node = next_node;
         next_node    = null;
      } while (current_node);
   }
};