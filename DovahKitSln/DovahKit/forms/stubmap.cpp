#include "stubmap.h"
#include <cstdarg>

bool FormStubMap::_move_up_struct::contains(FormStubMap::node& other) const {

}


bool FormStubMap::_apply_move_up_rule(node& x, node& w, _move_up_struct& out) {
   auto current = std::thread::id();
   //
   bool w_has  = w.lock.owner       != current;
   bool wl_has = w.left.lock.owner  != current;
   bool wr_has = w.right.lock.owner != current;
   bool wr_consistent = w.lock.owner == w.right.lock.owner;
   //
   bool a = (w.lock.owner == w.parent.lock.owner && wr_consistent && w_has && wl_has);
   bool b = (wr_consistent && w_has && wl_has);
   bool c = (!w_has && wl_has && wr_has);
   //
   if (a || b || c) {
      //
      // TODO: modify (out) to list the nodes we hold flags on
      //
      out.owner = w.right.lock.owner;
      return true;
   }
   return false;
}
bool FormStubMap::_spacing_rule_met(node& t, node& z, std::thread::id defer_to) {
   if (t != z)
      if (t.lock.owner != std::thread::id())
         return false;
   auto& tp = t.parent;
   if (tp != z) {
      if (move_data.contains(tp) && !tp.try_lock())
         return false;
      if (tp != t.parent) {
         tp.lock.lock = false;
         return false;
      }
      if (tp.lock.owner != std::thread::id()) {
         tp.lock.unlock();
         return false;
      }
   }
   auto& ts = (t == tp.left) ? tp.right : tp.left;
   if (!move_data.contains(ts) && !ts.try_lock()) {
      if (tp != z)
         this->_release_flags(move_data, false, tp);
      return false;
   }
   if (ts.lock.owner != std::thread::id() && ts.lock.owner != defer_to) {
      this->_release_flags(move_data, false, ts);
      if (tp != z)
         this->_release_flags(move_data, false, tp);
      return false;
   }
   if (tp != z)
      this->_release_flags(move_data, false, tp);
   this->_release_flags(move_data, false, ts);
   return true;
}
void FormStubMap::_release_flags(_move_up_struct& move_data, bool success, int node_count, ...) {
   va_list args;
   va_start(args, node_count);
   if (success) {
      for (int i = 0; i < node_count; i++) {
         auto node = va_arg(args, FormStubMap::node*);
         if (!move_data.contains(*node))
            node->unlock();
         else {
            if (node == move_data.goal) {
               //
               // TODO: release unneeded flags in move_data and discard it.
               //
               break;
            }
         }
      }
   } else {
      for (int i = 0; i < node_count; i++) {
         auto node = va_arg(args, FormStubMap::node*);
         if (!move_data.contains(*node))
            node->unlock();
      }
   }
   va_end(args);
}
bool FormStubMap::_get_flags_for_markers(node& start, _move_up_struct& move_data, node& pos1, node& pos2, node& pos3, node& pos4) {
   pos1 = start.parent;
   if (!move_data.contains(pos1) && !pos1.try_lock())
      return false;
   if (pos1 != start.parent) {
      this->_release_flags(move_data, false, 1, pos1);
      return false;
   }
   //
   pos2 = pos1.parent;
   if (!move_data.contains(pos2) && !pos2.try_lock()) {
      pos1.unlock();
      return false;
   }
   if (pos2 != pos1.parent) {
      this->_release_flags(move_data, false, 2, pos2, pos1);
      return false;
   }
   //
   pos3 = pos2.parent;
   if (!move_data.contains(pos3) && !pos3.try_lock()) {
      pos1.unlock();
      pos2.unlock();
      return false;
   }
   if (pos3 != pos2.parent) {
      this->_release_flags(move_data, false, 3, pos3, pos2, pos1);
      return false;
   }
   //
   pos4 = pos3.parent;
   if (!move_data.contains(pos4) && !pos4.try_lock()) {
      pos1.unlock();
      pos2.unlock();
      pos3.unlock();
      return false;
   }
   if (pos4 != pos3.parent) {
      this->_release_flags(move_data, false, 4, pos4, pos3, pos2, pos1);
      return false;
   }
   //
   return true;
}
bool FormStubMap::_get_flags_and_markers_above(node& start, int additional) {
   _move_up_struct move_data;
   node& pos1 = this->dummy;
   node& pos2 = this->dummy;
   node& pos3 = this->dummy;
   node& pos4 = this->dummy;
   if (!this->_get_flags_for_markers(start, move_data, pos1, pos2, pos3, pos4))
      return false;
   auto& first_new = pos4.parent;
   if (!move_data.contains(first_new) && !first_new.try_lock()) {
      this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
      return false;
   }
   if (first_new != pos4.parent && !this->_spacing_rule_met(first_new, start, PIDtoIgnore, move_data)) {
      this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
      return false;
   }
   auto& second_new = this->dummy;
   if (additional == 2) {
      second_new = first_new.parent;
      if (!move_data.contains(second_new) && !second_new.try_lock()) {
         this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
         return false;
      }
      if (second_new != first_new.parent && !this->_spacing_rule_met(second_new, start, PIDtoIgnore, move_data)) {
         this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
         return false;
      }
   }
   first_new.lock.owner = std::this_thread::get_id();
   if (additional == 2) {
      second_new.lock.owner = first_new.lock.owner;
      this->_release_flags(move_data, true, second_new);
   }
   this->_release_flags(move_data, true, first_new, pos4, pos3);
   if (additional == 1)
      this->_release_flags(move_data, true, pos2);
   return true;
}
bool FormStubMap::_prepare_insert(node& z) {
   auto& zp = z.parent;
   if (!zp.try_lock())
      return false;
   if (zp != z.parent) {
      zp.unlock();
      return false;
   }
   node& uncle = (z == z.parent.left) ? z.parent.right : z.parent.left;
   if (!uncle.try_lock()) {
      zp.unlock();
      return false;
   }
   if (!this->_get_flags_and_markers_above(zp, z)) {
      zp.unlock();
      uncle.unlock();
      return false;
   }
   return true;
}

void FormStubMap::insert(node& x) {
   auto& z = this->dummy;
   auto& y = this->root;
   while (y != this->dummy) {
      z = y;
      if (x < y)
         y = y.left;
      else
         y = y.right;
   }
   x.parent = z;
   if (z == this->dummy) {
      this->root = x;
   } else if (x < z)
      z.left = x;
   else
      z.right = x;
   x.left = this->dummy;
   x.right = this->dummy;
   x.color = node_color::red;
   this->_insert_fixup(x);
}
void FormStubMap::remove(node& z) {
   node& y = this->dummy;
   if (!z.left || !z.right) {
      y = z;
   } else {
      y = SUCCESSOR(z); // next key
   }
   node& x = y.left ? y.left : y.right;
   x.parent = y.parent;
   if (!y.parent)
      this->root = x;
   else {
      if (y == y.parent.left)
         y.parent.left = x;
      else
         y.parent.right = x;
   }
   if (y != z)
      z.data = y.data;
   if (y.color == node_color::black)
      this->_delete_fixup(x);
}
void FormStubMap::_delete_fixup(node& x) {
   while (x != this->root && x.color == node_color::black) {
      if (x == x.parent.left) {
         auto& w = x.parent.right;
         if (w.color == node_color::red) {
            w.color = node_color::black;
            x.parent.color = node_color::red;
            this->rotate_left(x.parent);
            w = x.parent.right;
         }
         if (w.left.color == node_color::black && x.right.color == node_color::black) {
            w.color = node_color::red;
            x = x.parent;
         } else {
            if (w.right.color == node_color::black) {
               x.left.color = node_color::black;
               w.color = node_color::red;
               this->rotate_right(w);
               w = x.parent.right;
            }
            w.color = x.parent.color;
            x.parent.color = node_color::black;
            w.right.color = node_color::black;
            this->rotate_left(x.parent);
            x = this->root;
         }
      } else
         x.parent = x.parent.parent.right;
   }
   x.color = node_color::black;
}
void FormStubMap::_insert_fixup(node& x) {
   node& p = this->dummy;
   while ((p = x.parent).color == node_color::red) {
      auto& gp = p.parent;
      if (p == gp.left) {
         auto& y = gp.right;
         if (y.color == node_color::red) {
            p.color = node_color::black;
            y.color = node_color::black;
            gp.color = node_color::red;
            //x = gp;
            x = this->_move_inserter_up(x);
         } else {
            if (x == p.right) {
               x = p;
               this->rotate_left(x);
               p = x.parent;
               gp = p.parent;
            }
            p.color = node_color::black;
            gp.color = node_color::red;
            this->rotate_right(gp);
         }
      } else
         x.parent = gp.right;
   }
   this->root.color = node_color::black;
}
void FormStubMap::rotate_right(node& y) {
   auto& x = y.left;
   y.left = x.right;
   x.right.parent = y;
   x.parent = y.parent;
   if (y.parent == this->root)
      this->root = x;
   else if (y == y.parent.left)
      y.parent.left = x;
   else
      y.parent.right = x;
   x.right = y;
   y.parent = x;
}
void FormStubMap::rotate_left(node& x) {
   auto& y = x.right;
   x.right = y.left;
   y.left.parent = x;
   y.parent = x.parent;
   if (!x.parent) {
      this->root = y;
   } else if (x == x.parent.left)
      x.parent.left = y;
   else
      x.parent.right = y;
   y.left = x;
   x.parent = y;
}