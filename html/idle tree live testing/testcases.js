
class Testcase {
   #forms_by_editor_id = new Map();
   
   constructor(content) {
      this.actions = [];
      this.idles   = [];
      
      if (content.actions) {
         for(let src of content.actions) {
            let id   = src.editor_id;
            let form = this.#forms_by_editor_id.get(id);
            if (form) {
               console.assert(form instanceof ActionForm);
            } else {
               form = new ActionForm(id);
               this.#forms_by_editor_id.set(id, form);
               this.actions.push(form);
            }
         }
      }
      if (content.idles) {
         for(let src of content.idles) {
            let id   = src.editor_id;
            let form = this.#forms_by_editor_id.get(id);
            if (form) {
               console.assert(form instanceof IdleForm);
            } else {
               form = new IdleForm({ editor_id: id });
               this.#forms_by_editor_id.set(id, form);
               this.idles.push(form);
            }
         }
         for(let src of content.idles) {
            let form = this.#forms_by_editor_id.get(src.editor_id);
            this.#parse_subrecord_list(form, src.subrecords.masters, true);
            this.#parse_subrecord_list(form, src.subrecords.active,  false);
         }
      }
   }
   
   #parse_subrecord_list(form, subrecords, is_master) {
      let graph = "";
      for(let subrecord of subrecords) {
         switch (subrecord.signature) {
            case "DATA":
               form.flags.is_parent = !!subrecord.is_parent;
               break;
            case "DNAM":
               graph = subrecord.string;
               break;
            case "ANAM":
               {
                  let dst_list = is_master ? form.serialized.masters : form.serialized.active;
                  let dst_item = new IdleSerialized();
                  dst_list.push(dst_item);
                  dst_item.graph    = graph;
                  dst_item.parent   = null;
                  dst_item.previous = null;
                  if (subrecord.parent) {
                     dst_item.parent = this.#forms_by_editor_id.get(subrecord.parent) || null;
                  }
                  if (subrecord.previous) {
                     dst_item.previous = this.#forms_by_editor_id.get(subrecord.previous) || null;
                     if (dst_item.previous && !(dst_item.previous instanceof IdleForm)) {
                        throw new Error("Invalid testcase");
                     }
                  }
               }
               break;
         }
      }
   }
};


const TESTCASES = {};

TESTCASES.typical_tree = new Testcase({
   /*
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
                - HumanActivateVariant01
                - HumanActivateVariant02
          - ActionDeath
             - HumanDeathRoot
       - Dog.hkx
          - ActionActivate
             - DogActivateRoot
   */
   actions: [
      { editor_id: "ActionActivate" },
      { editor_id: "ActionDeath" },
   ],
   idles: [
      {  // HumanActivateRoot
         editor_id: "HumanActivateRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // HumanActivateVariant01
         editor_id: "HumanActivateVariant01",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null }
            ],
         },
      },
      {  // HumanActivateVariant02
         editor_id: "HumanActivateVariant02",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivateVariant01" }
            ],
         },
      },
      {  // HumanDeathRoot
         editor_id: "HumanDeathRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null }
            ],
         },
      },
      {  // DogActivateRoot
         editor_id: "DogActivateRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test01_Dog.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // DogLooseBark
         editor_id: "DogLooseBark",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test01_Dog.hkx" },
               { signature: "ANAM", parent: null, previous: null }
            ],
         },
      },
   ],
});

TESTCASES.typical_sibling_ordering = new Testcase({
   /*
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
                - HumanActivateVariant01
                - HumanActivateVariant02
                - HumanActivateVariant03
      
      Testcase verifies correct sibling ordering post-build even when 
      siblings are seen out of order.
   */
   actions: [
      { editor_id: "ActionActivate" },
   ],
   idles: [
      {  // HumanActivateRoot
         editor_id: "HumanActivateRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test02_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // HumanActivateVariant03
         editor_id: "HumanActivateVariant03",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test02_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivateVariant02" }
            ],
         },
      },
      {  // HumanActivateVariant02
         editor_id: "HumanActivateVariant02",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test02_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivateVariant01" }
            ],
         },
      },
      {  // HumanActivateVariant01
         editor_id: "HumanActivateVariant01",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test02_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null }
            ],
         },
      },
   ],
});

TESTCASES.displaced_root = new Testcase({
   /*
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot_Displaced  [loaded earlier]
             - HumanActivateRoot_Displacing [loaded later]
      
      Testcase verifies that we correctly handle displacement of an idle 
      from the action for which it is a root.
   */
   actions: [
      { editor_id: "ActionActivate" },
   ],
   idles: [
      {  // HumanActivateRoot_Displaced
         editor_id: "HumanActivateRoot_Displaced",
         subrecords: {
            masters: [
               { signature: "DNAM", string: "Test03_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
            active: [
            ],
         },
      },
      {  // HumanActivateRoot_Displacing
         editor_id: "HumanActivateRoot_Displacing",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test03_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
   ],
});

TESTCASES.displaced_root_across_graphs = new Testcase({
   /*
      Testcase verifies that we correctly handle displacement of an action 
      root, when that action root has been moved across graphs and is being 
      displaced from a graph it is no longer in. Specifically:
      
       - The "displaced" idle is originally an action root in Human.
       
       - The "displaced" idle is overridden to be an action root in Dog, 
         such that it is now in two places at once and its canonical graph 
         is Dog.
         
       - The "displacing" idle is the same action root in Dog.
      
      The correct result should be that the "displaced" idle is both an 
      action root in Human and a loose idle in Dog, while the "displacing" 
      idle is an action root in Dog.
   */
   actions: [
      { editor_id: "ActionActivate" },
   ],
   idles: [
      {  // HumanActivateRoot_Displaced
         editor_id: "HumanToDogActivateRoot_Displaced",
         subrecords: {
            masters: [
               { signature: "DNAM", string: "Test04_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
               { signature: "DNAM", string: "Test04_Dog.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
            ],
            active: [
            ],
         },
      },
      {  // HumanActivateRoot_Displacing
         editor_id: "DogActivateRoot_Displacing",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test04_Dog.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
   ],
});