
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
   
   /*Testcase*/ clone() /*const*/ {
      let flattened = {
         actions: [],
         idles:   [],
      };
      this.#forms_by_editor_id.forEach(function(form, editor_id) {
         if (form instanceof ActionForm) {
            flattened.actions.push({
               editor_id: form.editor_id
            });
            return;
         }
         if (form instanceof IdleForm) {
            let item = {
               editor_id: form.editor_id,
               subrecords: {
                  masters: [],
                  active:  []
               }
            };
            for(let src of form.serialized.masters) {
               item.subrecords.masters.push({
                  signature: "DNAM",
                  string:    src.graph
               });
               item.subrecords.masters.push({
                  signature: "ANAM",
                  parent:    src.parent   ? src.parent.editor_id   : null,
                  previous:  src.previous ? src.previous.editor_id : null,
               });
            }
            if (form.flags.is_forced_loose) {
               item.subrecords.active.push({
                  signature:       "DATA",
                  is_forced_loose: true,
               });
            }
            for(let src of form.serialized.active) {
               item.subrecords.active.push({
                  signature: "DNAM",
                  string:    src.graph
               });
               item.subrecords.active.push({
                  signature: "ANAM",
                  parent:    src.parent   ? src.parent.editor_id   : null,
                  previous:  src.previous ? src.previous.editor_id : null,
               });
            }
            flattened.idles.push(item);
            return;
         }
      });
      return new Testcase(flattened);
   }
   
   #parse_subrecord_list(form, subrecords, is_master) {
      let graph = "";
      for(let subrecord of subrecords) {
         switch (subrecord.signature) {
            case "DATA":
               form.flags.is_forced_loose = !!subrecord.is_forced_loose;
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

TESTCASES.typical_tree = new Testcase({ // VERIFIED in CK
   /*
       - Dog.hkx
          - ActionActivate
             - DogActivateRoot
          - LOOSE
             - DogLooseBark
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
                - HumanActivateVariant01
                - HumanActivateVariant02
          - ActionDeath
             - HumanDeathRoot
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

TESTCASES.typical_tree_in_masters = new Testcase({
   /*
       - Dog.hkx
          - ActionActivate
             - DogActivateRoot
          - LOOSE
             - DogLooseBark
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
                - HumanActivateVariant01
                - HumanActivateVariant02
          - ActionDeath
             - HumanDeathRoot
   */
   actions: [
      { editor_id: "ActionActivate" },
      { editor_id: "ActionDeath" },
   ],
   idles: [
      {  // HumanActivateRoot
         editor_id: "HumanActivateRoot",
         subrecords: {
            active: [
            ],
            masters: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // HumanActivateVariant01
         editor_id: "HumanActivateVariant01",
         subrecords: {
            active: [
            ],
            masters: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null }
            ],
         },
      },
      {  // HumanActivateVariant02
         editor_id: "HumanActivateVariant02",
         subrecords: {
            active: [
            ],
            masters: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivateVariant01" }
            ],
         },
      },
      {  // HumanDeathRoot
         editor_id: "HumanDeathRoot",
         subrecords: {
            active: [
            ],
            masters: [
               { signature: "DNAM", string: "Test01_Human.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null }
            ],
         },
      },
      {  // DogActivateRoot
         editor_id: "DogActivateRoot",
         subrecords: {
            active: [
            ],
            masters: [
               { signature: "DNAM", string: "Test01_Dog.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // DogLooseBark
         editor_id: "DogLooseBark",
         subrecords: {
            active: [
            ],
            masters: [
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

TESTCASES.displaced_root = new Testcase({ // VERIFIED in CK
   /*
      Testcase verifies that we correctly handle displacement of an idle 
      from the action for which it is a root. We define the hierarchy as
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot_Displaced  [loaded earlier]
             - HumanActivateRoot_Displacing [loaded later]
      
      which should ultimately produce:
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot_Displacing
          - LOOSE
             - HumanActivateRoot_Displaced
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

//
// Invalid hierarchy tests.
//

TESTCASES.displaced_root_across_graphs = new Testcase({ // VERIFIED in CK
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
      idle is an action root in Dog:
      
       - Dog.hkx
          - ActionActivate
             - DogActivateRoot_Displacing
          - LOOSE
             - HumanToDogActivateRoot_Displaced [canonical]
       - Human.hkx
          - ActionActivate
             - HumanToDogActivateRoot_Displaced [non-canonical]
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

TESTCASES.action_root_becomes_child_idle = new Testcase({ // VERIFIED in CK
   /*
      Testcase for an idle being in two places at once: both the root of 
      an action, and the child of another idle. We test both an idle 
      doing this within a single graph (Human), and an idle doing this 
      across two graphs (from Cat to Dog). This test involves the roots 
      being initially placed by a master and the moved by the active 
      file.
      
       - Cat.hkx
          - ActionActivate
             - CatActivateRoot [non-canonical]
       - Dog.hkx
          - ActionDeath
             - DogDeathRoot
                - CatActivateRoot [canonical]
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot [non-canonical]
          - ActionDeath
             - HumanDeathRoot
                - HumanActivateRoot [canonical]
      
      The CK produces a matching tree, and additionally emits the following 
      warnings:
      
       - CatActivateRoot
          - Invalid parent idle
          - Parent array mismatch
       - HumanActivateRoot
          - Invalid parent idle
          - Parent array mismatch
   */
   actions: [
      { editor_id: "ActionActivate" },
      { editor_id: "ActionDeath" },
   ],
   idles: [
      {  // HumanDeathRoot
         editor_id: "HumanDeathRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test05_Human.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null }
            ],
         },
      },
      {  // HumanActivateRoot
         editor_id: "HumanActivateRoot",
         subrecords: {
            masters: [
               { signature: "DNAM", string: "Test05_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
            active: [
               { signature: "DNAM", string: "Test05_Human.hkx" },
               { signature: "ANAM", parent: "HumanDeathRoot", previous: null }
            ],
         },
      },
      {  // DogDeathRoot
         editor_id: "DogDeathRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test05_Dog.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null }
            ],
         },
      },
      {  // CatActivateRoot
         editor_id: "CatActivateRoot",
         subrecords: {
            masters: [
               { signature: "DNAM", string: "Test05_Cat.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
            active: [
               { signature: "DNAM", string: "Test05_Human.hkx" },
               { signature: "ANAM", parent: "DogDeathRoot", previous: null }
            ],
         },
      },
   ],
});

TESTCASES.cyclical_siblings = new Testcase({ // verified in CK
   /*
      Testcase for cyclical siblings. Final result should be:
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
               - HumanActivate01
               - HumanActivate02
               - HumanActivate05
          - LOOSE
             - HumanActivate03
             - HumanActivate04
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
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // HumanActivate01
         editor_id: "HumanActivate01",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null }
            ],
         },
      },
      {  // HumanActivate02
         editor_id: "HumanActivate02",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate01" }
            ],
         },
      },
      {  // HumanActivate03
         editor_id: "HumanActivate03",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate04" }
            ],
         },
      },
      {  // HumanActivate04
         editor_id: "HumanActivate04",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate03" }
            ],
         },
      },
      {  // HumanActivate05
         editor_id: "HumanActivate05",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate02" }
            ],
         },
      },
   ],
});

TESTCASES.siblings_with_different_parents = new Testcase({ // verified in CK
   /*
      Testcase for siblings with different parents. Final 
      result should be:
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
               - HumanActivate01
          - LOOSE
             - HumanActivate02
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
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // HumanActivate01
         editor_id: "HumanActivate01",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null }
            ],
         },
      },
      {  // HumanActivate02
         editor_id: "HumanActivate02",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: null, previous: "HumanActivate01" }
            ],
         },
      },
   ],
});

TESTCASES.siblings_with_just_one_odd_one_out = new Testcase({ // verified in CK
   /*
      Testcase for siblings with different parents. Final 
      result should be:
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
               - HumanActivate01
          - LOOSE
             - HumanActivate02
             - HumanActivate03
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
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null }
            ],
         },
      },
      {  // HumanActivate01
         editor_id: "HumanActivate01",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null }
            ],
         },
      },
      {  // HumanActivate02
         editor_id: "HumanActivate02",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: null, previous: "HumanActivate01" }
            ],
         },
      },
      {  // HumanActivate02
         editor_id: "HumanActivate03",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate02" }
            ],
         },
      },
   ],
});

TESTCASES.idle_in_multiple_active_file_roots = new Testcase({ // verified in CK
   /*
      Testcase for an idle that (by virtue of being malformed within 
      the active file) is placed in multiple action roots by the 
      active file.
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot [non-canonical]
          - ActionDeath
             - HumanActivateRoot [canonical]
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
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null },
            ],
         },
      },
   ],
});

TESTCASES.idle_in_active_idle_and_active_root = new Testcase({ // verified in CK
   /*
      Testcase for an idle that (by virtue of being malformed within 
      the active file) is placed in both an action root and a parent 
      idle by the active file.
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot [non-canonical]
          - ActionDeath
             - HumanDeathRoot
                - HumanActivateRoot [canonical]
      
      CK should trigger an identical-looking tree, and should also 
      emit the following warnings on HumanActivateRoot:
      
       - Invalid parent idle
       - Parent array mismatch
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
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "HumanDeathRoot", previous: null },
            ],
         },
      },
      {  // HumanDeathRoot
         editor_id: "HumanDeathRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null }
            ],
         },
      },
   ],
});

TESTCASES.action_root_moved_by_master_and_active = new Testcase({ // verified in CK
   /*
      Testcase for an idle that is placed by one master, moved by a 
      later-loaded master, and then moved to a third spot by the 
      active file.
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot [non-canonical]
          - ActionDeath
             - HumanActivateRoot [non-canonical]
          - ActionFall
             - HumanActivateRoot [canonical]
   */
   actions: [
      { editor_id: "ActionActivate" },
      { editor_id: "ActionDeath" },
      { editor_id: "ActionFall" },
   ],
   idles: [
      {  // HumanActivateRoot
         editor_id: "HumanActivateRoot",
         subrecords: {
            masters: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionDeath", previous: null },
            ],
            active: [
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionFall", previous: null },
            ],
         },
      },
   ],
});

TESTCASES.action_root_flagged_as_loose = new Testcase({ // verified in CK
   /*
      There's a flag in IDLE/DATA that causes the idle to always 
      be treated as loose, skipping all hierarchy processing. If 
      this flag is used on an action root, I'd expect that root 
      to end up in two places at once.
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot [non-canonical]
          - LOOSE
             - HumanActivateRoot [canonical]
   */
   actions: [
      { editor_id: "ActionActivate" },
   ],
   idles: [
      {  // HumanActivateRoot
         editor_id: "HumanActivateRoot",
         subrecords: {
            masters: [
               { signature: "DATA", is_forced_loose: true },
               { signature: "DNAM", string: "Test06_Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
            ],
            active: [],
         },
      },
   ],
});

TESTCASES.forced_loose_sibling = new Testcase({ // VERIFIED in CK
   /*
      Testcase verifies the effect of incorrectly applying the 
      "is forced loose" flag to an idle that would otherwise 
      be a non-loose sibling. The specified hierarchy is:
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
                - HumanActivate01
                - HumanActivate02 [is forced loose]
                - HumanActivate03
      
      The result hierarchy is:
      
       - Human.hkx
          - ActionActivate
             - HumanActivateRoot
                - HumanActivate01
                - HumanActivate03
          - LOOSE
                - HumanActivate02
      
      CK should trigger an identical-looking tree, and should also 
      emit the following warnings on HumanActivate03:
      
       - Invalid prev idle
   */
   actions: [
      { editor_id: "ActionActivate" },
   ],
   idles: [
      {
         editor_id: "HumanActivateRoot",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
            ],
         },
      },
      {
         editor_id: "HumanActivate01",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: null },
            ],
         },
      },
      {
         editor_id: "HumanActivate02",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DATA", is_forced_loose: true },
               { signature: "DNAM", string: "Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate01" },
            ],
         },
      },
      {
         editor_id: "HumanActivate03",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Human.hkx" },
               { signature: "ANAM", parent: "HumanActivateRoot", previous: "HumanActivate02" },
            ],
         },
      },
   ],
});

TESTCASES.forced_loose_runner_up = new Testcase({
   /*
      This testcase is meant for a specific situation during editing. 
      The "RunnerUp" idle is initially in two places at once, being 
      both an action root and a forced-loose idle, but it's displaced 
      from that action root by the "Displacing" idle, producing this 
      tree:
      
       - Human.hkx
          - ActionActivate
             - Displacing
          - LOOSE
             - RunnerUp
      
      Deleting the "Displacing" idle causes RunnerUp to no longer be 
      displaced; however, it's still forced-loose, so it should end 
      up in multiple places at once, forming this tree:
      
       - Human.hkx
          - ActionActivate
             - RunnerUp [non-canonical]
          - LOOSE
             - RunnerUp [canonical]
   */
   actions: [
      { editor_id: "ActionActivate" },
   ],
   idles: [
      {
         editor_id: "RunnerUp",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DATA", is_forced_loose: true },
               { signature: "DNAM", string: "Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
            ],
         },
      },
      {
         editor_id: "Displacing",
         subrecords: {
            masters: [
            ],
            active: [
               { signature: "DNAM", string: "Human.hkx" },
               { signature: "ANAM", parent: "ActionActivate", previous: null },
            ],
         },
      },
   ]
});

