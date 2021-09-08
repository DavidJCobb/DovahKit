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

      void userSubmitted(); // user pressed Ctrl + Enter

   protected:
      int  _maxLength = -1;
      bool _respondToChange = true; // use instead of QSignalBlocker
};