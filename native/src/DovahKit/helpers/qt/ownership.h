/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
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

   extern void move_object_backward(QObject* parent, QObject* subject);
   extern void move_object_forward(QObject* parent, QObject* subject);

   extern bool a_is_ancestor_of_b(const QObject* a, const QObject* b);
}
