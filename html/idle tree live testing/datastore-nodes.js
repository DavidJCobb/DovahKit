
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
};

class Action {
   constructor(/*String*/ editor_id) {
      this.graph     = null; // Graph
      this.editor_id = editor_id;
      this.root      = null; // Optional<Idle>
   }
   
   // for initial build only
   _track_candidate(/*IdleSerialized*/ candidacy, /*Idle*/ idle, /*bool*/ via_master) {
      throw new Error("NOT YET IMPLEMENTED");
   }
};

class IdleSerialized {
   constructor() {
      this.graph    = "";   // IDLE/DNAM
      this.parent   = null; // IDLE/ANAM: ActionForm or IdleForm
      this.previous = null; // IDLE/ANAM: ActionForm or IdleForm
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
      this.editor_id  = options.editor_id;  // String
      this.flags = {
         is_parent: false,
      };
      this.serialized = {
         masters: options.serialized.masters || [], // Array<IdleSerialized>
         active:  options.serialized.active  || [], // Array<IdleSerialized>
      };
   }
};

class Idle {
   constructor(/*String*/ editor_id) {
      this.editor_id  = editor_id;
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
      let _reverse_search = function(list) {
         const size = list.length;
         for(let i = size - 1; i > 0; --i) {
            let item = list[i];
            if (item.graph)
               return item.graph;
         }
      };
      let path = _reverse_search(this.serialized.active);
      if (!path)
         path = _reverse_search(this.serialized.masters);
      return path || "";
   }
   
   insert_sorted_child(/*Idle*/ idle) {
      // TODO
      throw new Error("NOT YET IMPLEMENTED");
   }
};