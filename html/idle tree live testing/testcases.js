
class TestIdle extends IdleForm {
   constructor(options) {
      super(options);
   }
   /*Idle*/ reify() {
      let _clone = function(src_list) {
         let dst_list = [];
         for(let src of src_list) {
            let dst = new IdleSerialized();
            dst.graph    = src.graph;
            dst.parent   = src.parent;
            dst.previous = src.previous;
            if (dst.parent instanceof Idle)
               dst.parent = dst.parent.editor_id;
            if (dst.previous instanceof Idle)
               dst.previous = dst.previous.editor_id;
            dst_list.push(dst);
         }
         return dst_list;
      };
      
      let idle = new Idle(this.editor_id);
      idle.serialized.masters = _clone(this.serialized.masters);
      idle.serialized.active  = _clone(this.serialized.active);
      return idle;
   }
};

class Testcase {
   constructor(
      /*Array<ActionForm>*/ actions,
      /*Array<IdleForm>*/   idles
   ) {
      this.actions = actions || [];
      this.idles   = idles  || [];
   }
};