
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
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "ActivateVariant01" }
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