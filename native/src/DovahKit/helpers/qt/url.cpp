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
#include "url.h"

namespace cobb::qt {
   extern QStringList split_url_path(const QUrl& url) {
      QString     path = url.path(QUrl::NormalizePathSegments | QUrl::StripTrailingSlash);
      QStringList list;
      int size = path.size();
      int from = 0;
      for (int i = 0; i < size; ++i) {
         QChar c = path[i];
         if (c == '/' || c == '\\') {
            if (i != 0)
               list.push_back(QStringRef(&path, from, i - from).toString());
            from = i + 1;
            continue;
         }
      }
      if (from < size)
         list.push_back(QStringRef(&path, from, size - from).toString());
      return list;
   }

   extern QVector<std::pair<QString, QString>> split_url_query(const QUrl& url) {
      QVector<std::pair<QString, QString>> out;
      QString query = url.query();
      int     size  = query.size();
      bool    key   = true;
      std::pair<QString, QString> current;
      //
      for (int i = 0; i <= size; ++i) {
         QChar c = i < size ? query[i] : QChar('&');
         if (key) {
            if (c == '&') {
               if (!current.first.isEmpty()) {
                  out.push_back(std::move(current));
               }
               continue;
            }
            if (c == '=') {
               key = false;
               continue;
            }
            current.first += c;
         } else {
            if (c == '&') {
               out.push_back(std::move(current));
               key = true;
               continue;
            }
            current.second += c;
         }
      }
      if (!current.first.isEmpty())
         out.push_back(std::move(current));
      return out;
   }

   extern bool url_is_web_scheme(const QUrl& url) {
      auto scheme = url.scheme();
      if (scheme.compare("http", Qt::CaseInsensitive) == 0)
         return true;
      if (scheme.compare("https", Qt::CaseInsensitive) == 0)
         return true;
      return false;
   }
}