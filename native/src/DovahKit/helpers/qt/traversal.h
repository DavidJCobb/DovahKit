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
#include <functional>
#include <QWidget>

namespace cobb::qt {
   extern void for_each_widget_in_hierarchy(QWidget* root, std::function<bool(QWidget*)> functor); // return (true) to stop early
   extern QWidget* topmost_container_of(QWidget*);

   // returns the nearest self-or-ancestor widget of the given type
   extern QWidget* nearest_widget_of_type(QWidget* base, const QMetaObject& type);

   extern bool object_is_or_contains(const QObject* haystack, const QObject* needle);
}
