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
#include <QPlainTextEdit>

class DovahscriptTextarea : public QPlainTextEdit {
   Q_OBJECT;
   public:
      DovahscriptTextarea(QWidget* parent = nullptr);

      inline int maxLength() const noexcept { return this->_maxLength; };
      void setMaxLength(int);

      // Replace the current plaintext, while preserving undo history and allowing the user 
      // to undo the replacement.
      void replaceTextWithUndo(const QString&);

   signals:
      void inputRejected();

      // This signal must be used instead of textChanged, as textChanged is fired by the base 
      // QPlainTextEdit internals before we have a chance to enforce the max length (and then 
      // fired again when we do enforce the max length).
      void inputAccepted(const QString&);

      void userSubmitted(const QString&); // user pressed Ctrl + Enter

   protected:
      int  _maxLength = -1;
      bool _respondToChange = true; // use instead of QSignalBlocker
};