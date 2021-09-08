#include "DovahscriptTextarea.h"
#include <QAction>
#include <QApplication>

DovahscriptTextarea::DovahscriptTextarea(QWidget* parent) : QPlainTextEdit(parent) {
   auto* doc = this->document();
   assert(doc);
   QObject::connect(doc, &QTextDocument::contentsChange, this, [this](int pos, int removed, int added) {
      if (!this->_respondToChange) // this is not user or programmatic input; we're modifying the document content in order to enforce the max length
         return;
      auto text = this->toPlainText();
      if (added - removed <= 0) {
         emit this->inputAccepted(text);
         return;
      }
      auto maxlength = this->maxLength();
      if (maxlength < 0) {
         emit this->inputAccepted(text);
         return;
      }
      if (text.size() <= maxlength) {
         emit this->inputAccepted(text);
         return;
      }
      //
      // Our current value exceeds the maximum length. Let's get rid of the previous input. The 
      // naive approach would be to truncate the string or use undo, but...
      // 
      //  - Truncating the string produces incorrect behavior if the user isn't actually typing 
      //    or pasting at the end of the string.
      // 
      //  - The "undo" function seems to act per-input. If the user holds down the "a" key until 
      //    they hit the length limit, "undo" will delete every typed "a" rather than just those 
      //    the one that pushed last the length limit.
      //
      auto* doc = this->document();
      auto  cur = QTextCursor(doc);
      cur.setPosition(pos);
      cur.setPosition(pos + added, QTextCursor::MoveMode::KeepAnchor);
      if (cur.hasSelection()) {
         cur.joinPreviousEditBlock(); // ensure that the next deleteChar call doesn't add to the undo history: make it part of the previous undo-able action, if any
         cur.deleteChar();
         cur.endEditBlock(); // joinPreviousEditBlock works by "reopening" the previous undo history entry, so now we need to close it again
         text = this->toPlainText();
      }
      if (text.size() > maxlength) { // failsafe
         text = text.left(maxlength);
         this->_respondToChange = false;
         this->setPlainText(text);
         this->_respondToChange = true;
      }
      emit this->inputRejected();
      QApplication::beep();
   });
   //
   // Ctrl + Enter support:
   //
   {
      auto* action = new QAction(this);
      action->setAutoRepeat(false);
      action->setShortcut(Qt::Key_Return | Qt::CTRL);
      connect(action, &QAction::triggered, this, &DovahscriptTextarea::userSubmitted);
      this->addAction(action);
   }
}

void DovahscriptTextarea::replaceTextWithUndo(const QString& text) {
   auto* doc = this->document();
   auto  cur = QTextCursor(doc);
   cur.select(QTextCursor::Document);
   cur.insertText(text);
}

void DovahscriptTextarea::setMaxLength(int length) {
   auto old = this->_maxLength;
   if (old == length)
      return;
   this->_maxLength = length;
   if (length < old) {
      auto text = this->toPlainText();
      if (text.size() > length) {
         this->_respondToChange = false;
         this->setPlainText(text.left(length));
         this->_respondToChange = true;
      }
   }
}