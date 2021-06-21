#include "ownership.h"

namespace cobb::qt {
   extern void move_object_before(QObject* parent, QObject* subject, QObject* target) {
      assert(subject != parent);
      assert(!a_is_ancestor_of_b(subject, parent)); // cannot make a QObject a child of its own descendant
      assert(target->parent() == parent);
      assert(!subject->isWidgetType() && "What are you doing, dear programmer? Widgets need special handling. Don't use this on them! (Consider QWidget::raise and friends.)");
      assert(!target->isWidgetType()  && "What are you doing, dear programmer? Widgets need special handling. Don't use this on them! (Consider QWidget::raise and friends.)");
      if (subject->parent() != parent)
         subject->setParent(parent);
      //
      // Qt doesn't have any built-in functionality for reordering a QObject's children. For QWidget 
      // children, you can call the child's "raise" or "lower" member functions, but that can't be 
      // done for QObjects. The function to get the child list returns a const reference to the real 
      // child list, so we'll just do this the stupid way: edit the list directly by abusing a const 
      // cast, and do what we need to do.
      //
      auto& list = const_cast<QObjectList&>(parent->children());
      int   from = list.indexOf(subject);
      int   to   = list.indexOf(target);
      list.move(from, to);
      assert(list.indexOf(subject) == to); // If this fails, then perhaps Qt changed in some way, e.g. having the children() getter copy the list to prevent tampering.
   }

   extern void move_object_after(QObject* parent, QObject* subject, QObject* target) {
      assert(subject != parent);
      assert(!a_is_ancestor_of_b(subject, parent)); // cannot make a QObject a child of its own descendant
      assert(target->parent() == parent);
      assert(!subject->isWidgetType() && "What are you doing, dear programmer? Widgets need special handling. Don't use this on them! (Consider QWidget::raise and friends.)");
      assert(!target->isWidgetType()  && "What are you doing, dear programmer? Widgets need special handling. Don't use this on them! (Consider QWidget::raise and friends.)");
      if (subject->parent() != parent)
         subject->setParent(parent);
      //
      // Qt doesn't have any built-in functionality for reordering a QObject's children. For QWidget 
      // children, you can call the child's "raise" or "lower" member functions, but that can't be 
      // done for QObjects. The function to get the child list returns a const reference to the real 
      // child list, so we'll just do this the stupid way: edit the list directly by abusing a const 
      // cast, and do what we need to do.
      //
      auto& list = const_cast<QObjectList&>(parent->children());
      int   from = list.indexOf(subject);
      int   to   = list.indexOf(target);
      if (from > to)
         ++to;
      list.move(from, to);
      assert(list.indexOf(subject) == to); // If this fails, then perhaps Qt changed in some way, e.g. having the children() getter copy the list to prevent tampering.
   }

   extern bool a_is_ancestor_of_b(const QObject* a, const QObject* b) {
      if (b == a)
         return false;
      while (b = b->parent())
         if (b == a)
            return true;
      return false;
   }
}
