#include "tree.h"
#include <algorithm> // std::swap
#include <bit> // std::bit_width
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "helpers/streams/bitreader.h"
#include "helpers/streams/bitwriter.h"
#include "./nodes/root.h"

namespace dovahkit::subsystems::worldinput2::binds {
   tree::tree(input_device_type d) : device_type(d) {
      this->root = new nodes::root;
   }
   
   tree::tree(const tree& o) {
      assert(o.root);
      this->name        = o.name;
      this->device_type = o.device_type;
      this->root        = (nodes::root*)o.root->clone();
   }
   tree& tree::operator=(const tree& o) {
      if (&o == this)
         return *this;
      assert(o.root);
      if (this->root)
         delete this->root;
      this->name        = o.name;
      this->device_type = o.device_type;
      this->root        = (nodes::root*)o.root->clone();
      return *this;
   }
   
   tree::tree(tree&& o) {
      std::swap(this->name, o.name);
      std::swap(this->device_type, o.device_type);
      std::swap(this->root,        o.root);
   }
   tree& tree::operator=(tree&& o) {
      if (&o == this)
         return *this;
      std::swap(this->name, o.name);
      std::swap(this->device_type, o.device_type);
      std::swap(this->root,        o.root);
      return *this;
   }

   tree::~tree() {
      if (this->root) {
         delete this->root;
         this->root = nullptr;
      }
   }

   bool tree::operator==(const tree& other) const {
      if (this->device_type != other.device_type)
         return false;

      if (this->name != other.name)
         return false;

      if (this->root) {
         if (!other.root)
            return false;
         if (*this->root != *other.root)
            return false;
      } else {
         if (other.root)
            return false;
      }

      return true;
   }

   /*static*/ tree tree::read(cobb::streams::bitreader& stream) {
      uint32_t version;
      stream.read(version);

      input_device_type device_type;
      stream.read(device_type);

      tree out = tree(device_type);

      stream.read(out.name);

      bool presence;
      stream.read(presence);
      if (presence) {
         auto* root = node::read(stream);
         out.root = (nodes::root*)root;
         assert(root);
         assert(root->type == node_type::root);
      }
      return out;
   }
   void tree::write(cobb::streams::bitwriter& stream) const {
      stream.write(
         serialization_version,
         this->device_type,
         this->name
      );
      stream.write(this->root != nullptr);
      if (this->root != nullptr) {
         this->root->write(stream);
      }
   }

   /*static*/ tree tree::read(cobb::bitstreams::reader& s) {
      input_device_type device_type;
      s.stream(device_type);

      tree out = tree(device_type);

      s.stream<std::bit_width(max_name_length)>(out.name);

      bool presence;
      s.stream(presence);
      if (presence) {
         auto* root = node::read(s);
         out.root = (nodes::root*)root;
         assert(root);
         assert(root->type == node_type::root);
      }
      return out;
   }
   void tree::write(cobb::bitstreams::writer& s) const {
      assert(s.get_bitpos() == 0);
      {
         auto& h = s.header();
         h.version = serialization_version;
         s.stream(h);
      }

      s.stream(this->device_type);
      s.stream<std::bit_width(max_name_length)>(this->name);
      s.stream(this->root != nullptr);
      if (this->root != nullptr) {
         this->root->write(s);
      }
   }
}