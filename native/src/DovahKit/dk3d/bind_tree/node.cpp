#include "node.h"

namespace DK3D::binds {
   node::node(node_type t) : type(t) {}
   node::~node() {
      for (auto* p : this->children)
         if (p)
            delete p;
      this->children.clear();
   }

   void node::append(node& o) {
      assert(o.type != node_type::root);
      if (o.parent) {
         if (o.parent == this)
            return;
         o.parent->remove(o);
      }
      o.parent = this;
      this->children.append(&o);
   }
   void node::insert(node& o, size_t before) {
      assert(o.type != node_type::root);
      if (o.parent) {
         if (o.parent == this)
            return;
         o.parent->remove(o);
      }
      o.parent = this;
      this->children.insert(before, &o);
   }
   void node::remove(node& o) {
      assert(o.type != node_type::root);
      if (o.parent != this)
         return;
      o.parent = nullptr;
      this->children.removeOne(&o);
   }
   node* node::clone() const {
      auto* copy = this->_clone_impl();
      auto& list = this->children;
      auto  size = list.size();
      copy->children.resize(size);
      for (decltype(size) i = 0; i < size; ++i) {
         auto* c = list[i]->clone();
         c->parent = copy;
         copy->children[i] = c;
      }
      return copy;
   }
}