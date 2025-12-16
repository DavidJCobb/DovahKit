#include "./branch_node.h"
#include <cassert>
#include "./passkeys/initial_build.h"

namespace dovah::datastores::impl::story_manager {
   void branch_node::_append_during_load(passkeys::initial_build, node& subject) {
      assert(subject.parent == nullptr);
      if (&subject == this)
         return;
      this->children.push_back(&subject);
      subject.parent = this;
   }
   void branch_node::_insert_during_load(passkeys::initial_build, node& subject, size_t at) {
      assert(subject.parent == nullptr);
      assert(at <= this->children.size());
      if (&subject == this)
         return;
      this->children.insert(this->children.begin() + at, &subject);
      subject.parent = this;
   }
   void branch_node::_insert_during_load(passkeys::initial_build passkey, node& subject, const node& after) {
      size_t i = this->index_of_child(after);
      if (i == index_of_none) {
         this->_append_during_load(passkey, subject);
         return;
      }
      this->_insert_during_load(passkey, subject, i + 1);
   }
}