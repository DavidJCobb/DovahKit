
class DatastoreElement extends HTMLElement {
   #datastore;
   #list;
   #shadow;
   
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
         shadow.append(list);
      }
   }
   
   get datastore() { return this.#datastore; }
   set datastore(v) {
      if (v === this.#datastore)
         return;
      if (v && !(v instanceof Datastore))
         throw new TypeError("invalid value");
      this.#datastore = v;
      this.#re_render();
   }
   
   #render_idle(idle) {
      let node = document.createElement("li");
      node.classList.add("idle");
      node.datastore_node = idle;
      {
         let name = document.createElement("label");
         name.textContent = idle.editor_id;
         node.append(name);
      }
      if (idle.live.children.length) {
         let list = document.createElement("ul");
         node.append(list);
         for(let child of idle.live.children) {
            list.append(this.#render_idle(child));
         }
      }
      return node;
   }
   
   #render_action(action) {
      let node = document.createElement("li");
      node.classList.add("action");
      {
         let name = document.createElement("label");
         name.textContent = action.editor_id;
         node.append(name);
      }
      let nest = document.createElement("ul");
      node.append(nest);
      if (action.root) {
         nest.append(this.#render_idle(action.root));
      }
      return node;
   }
   
   #render_loose(loose, title) {
      let node = document.createElement("li");
      {
         let name = document.createElement("label");
         name.textContent = title || "LOOSE";
         node.append(name);
      }
      let nest = document.createElement("ul");
      node.append(nest);
      for(let idle of loose.idles)
         nest.append(this.#render_idle(idle));
      return node;
   }
   
   #render_graph(graph) {
      let node = document.createElement("li");
      node.classList.add("graph");
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
      let frag = new DocumentFragment();
      
      for(let graph of this.#datastore.graphs) {
         let node = this.#render_graph(graph);
         frag.append(node);
      }
      frag.append(this.#render_loose(this.#datastore.loose, "ORPHANS"));
      
      this.#list.replaceChildren(frag);
   }
};
customElements.define("datastore-element", DatastoreElement);