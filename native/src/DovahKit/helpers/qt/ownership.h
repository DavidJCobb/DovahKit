#pragma once
#include <QObject>

namespace cobb::qt {
   // Insert (subject) into (parent)'s child list, before (target). Asserts that 
   // (target) is inside of (parent) and that neither of the two objects are 
   // widgets (which require special handling).
   //
   // You shouldn't use this for typical operations; one use case would be if 
   // you have a custom widget with QObject children, where the child order 
   // determines Z-order or something similar.
   extern void move_object_before(QObject* parent, QObject* subject, QObject* target);

   extern void move_object_after(QObject* parent, QObject* subject, QObject* target);

   extern bool a_is_ancestor_of_b(const QObject* a, const QObject* b);
}
