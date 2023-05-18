#include "./node.h"
#include "./nodes/abstract_input_node.h"

#include "helpers/streams/bitreader.h"
#include "helpers/streams/bitwriter.h"
#include "./nodes/root.h"
#include "./nodes/bound_tool.h"
#include "./nodes/editor_mode.h"
#include "./nodes/modifier.h"

namespace dovahkit::subsystems::worldinput2::binds {
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

   /*static*/ node* node::read(cobb::streams::bitreader& stream) {
      node_type type;
      stream.read(type);

      node* out = nullptr;
      switch (type) {
         using enum node_type;
         case root:
            out = new nodes::root;
            break;
         case bound_tool:
            out = new nodes::bound_tool;
            break;
         case editor_mode:
            out = new nodes::editor_mode;
            break;
         case modifier:
            out = new nodes::modifier;
            break;
      }
      out->_read_impl(stream);

      if (out->is_valid_parent()) {
         size_t size;
         stream.read(size);
         for (size_t i = 0; i < size; ++i) {
            auto* child = read(stream);
            out->children.append(child);
         }
      }
      return out;
   }
   void node::write(cobb::streams::bitwriter& stream) const {
      stream.write(this->type);
      this->_write_impl(stream);
      if (this->is_valid_parent()) {
         for (const auto* child : this->children)
            child->write(stream);
      }
   }
}