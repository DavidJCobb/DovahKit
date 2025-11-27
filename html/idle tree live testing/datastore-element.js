
class DatastoreElement extends HTMLElement {
   #allow_editing = true;
   #datastore;
   #list;
   #shadow;
   
   #datastore_nodes_to_dom_nodes = new Map();
   
   constructor() {
      super();
      const shadow = this.#shadow = this.attachShadow({ mode: "open" });
      {
         const link = document.createElement("link");
         link.setAttribute("rel", "stylesheet");
         link.setAttribute("href", "./datastore-element.css");
         shadow.append(link);
      }
      {
         const list = this.#list = document.createElement("ul");
         list.classList.add("allow-editing");
         shadow.append(list);
         list.addEventListener("click", this.#on_list_click.bind(this));
         list.addEventListener("dragstart", this.#on_dragstart.bind(this));
         list.addEventListener("dragover", this.#on_dragover.bind(this));
         list.addEventListener("dragend", this.#on_dragend.bind(this));
         list.addEventListener("drop", this.#on_drop.bind(this));
      }
   }
   
   get datastore() { return this.#datastore; }
   set datastore(v) {
      if (v === this.#datastore)
         return;
      if (v && !(v instanceof Datastore))
         throw new TypeError("invalid value");
      this.#datastore = v;
      
      if (v) {
         v.callbacks.node_moved.before = this.#on_before_node_moved.bind(this);
         v.callbacks.node_moved.after  = this.#on_after_node_moved.bind(this);
         v.callbacks.idle_becoming_multiply_present = this.#on_idle_becoming_multiply_present.bind(this);
         v.callbacks.idle_no_longer_multiply_present = this.#on_idle_no_longer_multiply_present.bind(this);
      }
      
      this.#re_render();
   }
   
   get allow_editing() { return this.#allow_editing; }
   set allow_editing(v) {
      v = !!v;
      if (this.#allow_editing === v)
         return;
      this.#allow_editing = v;
      this.#list.classList[v ? "add" : "remove"]("allow-editing");
      if (!v) {
         this.#stop_dragging_idle();
      }
   }
   
   get_all_forms() {
      let out = {
         actions: new Set(),
         idles:   [],
      };
      if (!this.#datastore)
         return out;
      for(let graph of this.#datastore.graphs) {
         for(let action of graph.actions) {
            out.actions.add(action.form);
         }
      }
      for(let idle of this.#datastore.idles_by_id.values()) {
         out.idles.push(idle.form);
      }
      out.actions = Array.from(out.actions);
      return out;
   }
   
   //
   // DATASTORE LIVE-EDIT CALLBACKS
   //
   
   #pending_ops = [];
   
   #on_before_node_moved(subject, dst, index) { // beginMoveRows
      console.assert(index >= 0);
      this.#pending_ops.push({
         type:        "move",
         subject:     subject,
         destination: dst,
         index:       index,
      });
   }
   #on_after_node_moved(subject) { // endMoveRows
      let op = this.#pending_ops[this.#pending_ops.length - 1];
      if (!op || op.type != "move" || op.subject != subject)
         throw new Error("mismatched callbacks!");
      
      let subject_node = this.#datastore_nodes_to_dom_nodes.get(subject);
      let dst_node     = this.#datastore_nodes_to_dom_nodes.get(op.destination).querySelector(":scope>ul.idle-list");
      let dst_index    = op.index;
      console.assert(!!dst_node);
      
      let prev = null;
      if (dst_index > 0)
         prev = dst_node.children[dst_index - 1];
      subject_node.remove();
      if (prev)
         prev.after(subject_node);
      else
         dst_node.prepend(subject_node);
   }
   #on_idle_becoming_multiply_present(idle, action) {
      console.assert(idle.live.parent !== action);
      let dom_action = this.#datastore_nodes_to_dom_nodes.get(action);
      let list       = dom_action.querySelector(":scope>ul.idle-list");
      console.assert(!list.querySelector("li.idle"));
      list.append(this.#render_idle(idle, action));
   }
   #on_idle_no_longer_multiply_present(idle, action) {
      let dom_action = this.#datastore_nodes_to_dom_nodes.get(action);
      let dom_idle   = null;
      for(let item of dom_action.querySelector(":scope>ul.idle-list").children) {
         if (item.datastore_node == idle) {
            dom_idle = item;
            break;
         }
      }
      console.assert(!!dom_idle);
      console.assert(dom_idle.classList.contains("multiply-placed"));
      dom_idle.remove();
   }
   
   //
   // JS INTERACTION EVENT HANDLERS
   //
   
   #on_list_click(e) {
      if (!this.#allow_editing)
         return;
      let button = e.target.closest(".actions>button");
      if (button && this.#list.contains(button)) {
         if (button.classList.contains("delete")) {
            let item = button.closest(".idle");
            console.assert(item && item.datastore_node);
            this.#delete_idle(item.datastore_node);
            return;
         }
      }
   }
   
   //
   // JAVASCRIPT DRAG-AND-DROP UI DROSS
   //
   
   #on_dragstart(e) {
      if (!this.#allow_editing)
         return true;
      let node = e.target.closest(".idle");
      if (!node || !this.#list.contains(node))
         return true;
      if (node.classList.contains("multiply-placed"))
         return true;
      e.dataTransfer.items.add(node.datastore_node.form.editor_id, "dovahkit/dragged-idle");
      e.dataTransfer.effectAllowed = "move";
      
      // Chromium bug: changing the DOM in any way during `dragstart` causes 
      // Chromium to immediately fire `dragend`.
      window.setTimeout(
         this.#start_dragging_idle.bind(this, node.datastore_node),
         1
      );
   }
   #drag_event_is_for_idle(e) {
      //
      // This has to be a separate function from `idle_from_drag_event` because 
      // the DataTransferItem is not readable outside of `dragstart` and `drop`. 
      // This is extremely important to know, so naturally, no one bothered to 
      // document it on any mainstream reference sites, and you have to go read 
      // the spec to find it out.
      //
      // Attempting to call `DataTransferItem.prototype.getAsString` outside of 
      // those two events will cause the call to silently fail (your callback is 
      // never invoked).
      //
      for(let item of e.dataTransfer.items)
         if (item.type == "dovahkit/dragged-idle")
            return true;
   }
   #idle_from_drag_event(e) {
      if (!this.#datastore)
         return Promise.resolve(null);
      let result = Promise.withResolvers();
      for(let item of e.dataTransfer.items) {
         if (item.type == "dovahkit/dragged-idle") {
            //
            // Retrieving string-format drag data is a faux-async API. It doesn't 
            // return a promise, as all modern async web APIs do; instead, it 
            // invokes a callback on the next script tick if you're currently 
            // allowed to read the data (see above). If you're *not* currently 
            // allowed to read the data, then it silently fails with absolutely 
            // zero recourse for you as the API user.
            // 
            // This, too, is not covered by any mainstream reference sites, which 
            // all demonstrate usage but make no attempt to clarify the timing 
            // with which the callback is invoked.
            //
            item.getAsString((function(data) {
               result.resolve(this.#datastore.idles_by_id.get(data));
            }).bind(this));
            break;
         }
      }
      return result.promise;
   }
   #on_dragover(e) {
      if (!this.#drag_event_is_for_idle(e))
         return true;
      let node = e.target.closest(".action, .idle, .idle-adjacent-drop-target");
      if (!node || !this.#list.contains(node))
         return true;
      e.preventDefault();
      e.dataTransfer.dropEffect = "move";
   }
   #on_dragend(e) {
      if (this.#drag_event_is_for_idle(e))
         this.#stop_dragging_idle();
   }
   async #on_drop(e) {
      let idle = await this.#idle_from_drag_event(e);
      if (!idle)
         return;
      e.preventDefault();
      
      // `e.target` is always the containing custom element; the native systems 
      // that power the Drag and Drop API never try to look any deeper into a 
      // shadow root, even if it's an open shadow root. This means that the 
      // only way to figure out the precise drop target within a shadow root is 
      // via the non-standard ShadowRoot.prototype.elementFromPoint API. (This 
      // API is also present on Document and is standard there, but that, too, 
      // doesn't seem to ever peer into an open shadow root.)
      let target = this.#shadow.elementFromPoint(e.clientX, e.clientY);
      
      let node = target?.closest(".action, .idle, .idle-adjacent-drop-target");
      if (!node || !this.#list.contains(node)) {
         this.#stop_dragging_idle();
         return true;
      }
      
      let dst_parent   = null;
      let dst_previous = null;
      if (node.matches(".idle-adjacent-drop-target")) {
         dst_parent   = node.closest(".action, .idle, .loose").datastore_node;
         dst_previous = node.previousElementSibling?.datastore_node;
      } else {
         dst_parent = node.datastore_node;
      }
      this.#stop_dragging_idle();
      if (!this.#allow_editing)
         return;
      this.#move_idle(idle, dst_parent, dst_previous);
   }
   
   // JS-specific UI handler
   #start_dragging_idle(/*Idle*/ idle) {
      this.classList.add("moving-idle");
      //
      // Create drop targets for placing idles between each other.
      //
      this.#list.querySelectorAll("ul.idle-list").forEach(function(list) {
         let idles = Array.from(list.children);
         for(let idle of idles) {
            let item = document.createElement("div");
            item.classList.add("idle-adjacent-drop-target");
            idle.after(item);
         }
         let item = document.createElement("div");
         item.classList.add("idle-adjacent-drop-target");
         list.prepend(item);
      });
   }
   
   // JS-specific UI handler
   #stop_dragging_idle() {
      this.classList.remove("moving-idle");
      this.#list.querySelectorAll(".idle-adjacent-drop-target").forEach(function(node) {
         node.remove();
      });
   }
   
   //
   // ACTUAL DATA-EDITING OPERATIONS
   //
   
   #move_idle(/*Idle*/ subject, /*Variant<Idle, LooseIdleList, Action>*/ dst_parent, /*Optional<Idle>*/ dst_after) {
      this.#datastore.move_idle(subject, dst_parent, dst_after);
      
      let event = new CustomEvent("datastore-change");
      this.dispatchEvent(event);
   }
   
   #delete_idle(/*Idle*/ subject) {
      // TODO
      alert("NOT YET IMPLEMENTED");
      throw new Error("NOT YET IMPLEMENTED");
   }
   
   //
   // RENDERING
   //
   
   #associate_dom(datastore_node, dom_node) {
      this.#datastore_nodes_to_dom_nodes.set(datastore_node, dom_node);
      dom_node.datastore_node = datastore_node;
   }
   
   #build_idle_action_buttons() {
      let node = document.createElement("div");
      node.classList.add("actions");
      {
         let button = document.createElement("button");
         button.textContent = "Delete";
         button.classList.add("delete");
         node.append(button);
      }
      return node;
   }
   
   #render_idle(idle, via_parent) {
      let node = document.createElement("li");
      node.classList.add("idle");
      node.setAttribute("draggable", "true");
      if (idle.live.parent != via_parent) {
         node.classList.add("multiply-placed");
         node.datastore_node = idle;
      } else {
         this.#associate_dom(idle, node);
      }
      {
         let name = document.createElement("label");
         name.textContent = idle.editor_id;
         node.append(name);
      }
      node.append(this.#build_idle_action_buttons());
      if (idle.live.children.length) {
         let list = document.createElement("ul");
         node.append(list);
         for(let child of idle.live.children) {
            list.append(this.#render_idle(child, idle));
         }
      }
      return node;
   }
   
   #render_action(action) {
      let node = document.createElement("li");
      node.classList.add("action");
      this.#associate_dom(action, node);
      {
         let name = document.createElement("label");
         name.textContent = action.editor_id;
         node.append(name);
      }
      let nest = document.createElement("ul");
      nest.classList.add("idle-list");
      node.append(nest);
      if (action.root) {
         nest.append(this.#render_idle(action.root, action));
      }
      return node;
   }
   
   #render_loose(loose, title) {
      let node = document.createElement("li");
      node.classList.add("loose");
      this.#associate_dom(loose, node);
      {
         let name = document.createElement("label");
         name.textContent = title || "LOOSE";
         node.append(name);
      }
      let nest = document.createElement("ul");
      nest.classList.add("idle-list");
      node.append(nest);
      for(let idle of loose.idles)
         nest.append(this.#render_idle(idle, loose));
      return node;
   }
   
   #render_graph(graph) {
      let node = document.createElement("li");
      node.classList.add("graph");
      this.#associate_dom(graph, node);
      {
         let name = document.createElement("label");
         name.textContent = graph.path;
         node.append(name);
      }
      let list = document.createElement("ul");
      node.append(list);
      for(let action of graph.actions) {
         list.append(this.#render_action(action));
      }
      list.append(this.#render_loose(graph.loose));
      return node;
   }
   
   #re_render() {
      this.#datastore_nodes_to_dom_nodes = new Map();
      
      let frag = new DocumentFragment();
      if (!this.#datastore) {
         this.#list.replaceChildren(frag);
         return;
      }
      
      for(let graph of this.#datastore.graphs) {
         let node = this.#render_graph(graph);
         frag.append(node);
      }
      let loose = this.#render_loose(this.#datastore.loose, "ORPHANS");
      loose.classList.add("orphans");
      frag.append(loose);
      
      this.#list.replaceChildren(frag);
   }
};
customElements.define("datastore-element", DatastoreElement);