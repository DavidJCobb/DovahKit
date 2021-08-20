#include "DKLuaSyntaxHighlighter.h"
#include <array>
#include <QRegularExpressionMatchIterator>
#include <QString>

namespace {
   const std::array keywords = {
      QString("and"),
      QString("break"),
      QString("do"),
      QString("for"),
      QString("function"),
      QString("else"),
      QString("elseif"),
      QString("end"),
      QString("false"),
      QString("if"),
      QString("in"),
      QString("local"),
      QString("nil"),
      QString("not"),
      QString("or"),
      QString("repeat"),
      QString("return"),
      QString("then"),
      QString("true"),
      QString("until"),
      QString("while"),
   };

   static bool is_keyword_boundary(QChar c) {
      if (c == '_')
         return false;
      if (c.isLetterOrNumber())
         return false;
      return true;
   }
}

DKLuaSyntaxHighlighter::DKLuaSyntaxHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {
   this->formats.comment.block.setFontItalic(true);
   this->formats.comment.block.setForeground(QColor::fromRgb(48, 192, 8));
   this->formats.comment.line.setFontItalic(true);
   this->formats.comment.line.setForeground(QColor::fromRgb(48, 192, 8));
   this->formats.keyword.setFontWeight(QFont::Weight::Bold);
   this->formats.keyword.setForeground(QColor::fromRgb(0, 16, 255));
   this->formats.string.setForeground(QColor::fromRgb(140, 140, 140));
}
void DKLuaSyntaxHighlighter::highlightBlock(const QString& text) {
   int state = this->previousBlockState();
   int start = 0;
   //
   if (state == BlockState::None) {
      for (auto& keyword : keywords) {
         if (text == keyword) {
            this->setFormat(0, text.size(), this->formats.keyword);
            return;
         }
      }
   }
   //
   for (int i = 0; i < text.length(); ++i) {
      switch (state) {
         case BlockState::Comment_Block:
            if (text.mid(i, 4) == "]]--") {
               state = BlockState::None;
               this->setFormat(start, i - start + 4, this->formats.comment.block);
               //
               // Skip ahead by the size of the delimiter (minus one because the for-loop will 
               // also increment us), so that the end of a block comment isn't also treated as 
               // the start of a line comment.
               //
               i += 4 - 1;
            }
            break;
         case BlockState::Comment_Line:
            if (text[i] == '\n') {
               state = BlockState::None;
               this->setFormat(start, i - start, this->formats.comment.line);
            }
            break;
         case BlockState::String_Double:
            if (text[i] == '"') {
               state = BlockState::None;
               this->setFormat(start, i - start + 1, this->formats.string);
            }
            break;
         case BlockState::String_Single:
            if (text[i] == '\'') {
               state = BlockState::None;
               this->setFormat(start, i - start + 1, this->formats.string);
            }
            break;
         default:
            if (text[i] == '"') {
               state = BlockState::String_Double;
               start = i;
               break;
            }
            if (text[i] == '\'') {
               state = BlockState::String_Single;
               start = i;
               break;
            }
            if (text.mid(i, 4) == "--[[") {
               state = BlockState::Comment_Block;
               start = i;
               break;
            }
            if (text.mid(i, 2) == "--") {
               state = BlockState::Comment_Line;
               start = i;
               break;
            }
            if (is_keyword_boundary(text[i])) {
               // search backward for a keyword
               QStringRef view = QStringRef(&text, 0, i);
               for (auto& keyword : keywords) {
                  if (view.endsWith(keyword)) {
                     auto size    = keyword.size();
                     bool bounded = true;
                     if (size > i) {
                        bounded = is_keyword_boundary(text[i - size]);
                     }
                     if (bounded) {
                        this->setFormat(i - size, size, this->formats.keyword);
                     }
                  }
               }
            }
            break;
      }
   }
   switch (state) {
      case BlockState::Comment_Block:
         this->setFormat(start, text.size() - start, this->formats.comment.block);
         break;
      case BlockState::Comment_Line:
         this->setFormat(start, text.size() - start, this->formats.comment.line);
         //
         // Qt's documentation is not at all clear about this, but a "block" is never more than one 
         // line; ergo any states that must terminate at the end of a line should not be stored.
         //
         [[fallthrough]];
      case BlockState::String_Double:
         [[fallthrough]];
      case BlockState::String_Single:
         state = BlockState::None;
         break;
   }
   setCurrentBlockState(state);
}