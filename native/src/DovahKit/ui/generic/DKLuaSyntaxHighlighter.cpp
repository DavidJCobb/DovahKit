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
      QString("goto"),
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

   const std::array operators = {
      QString("="),
      QString("+="),
      QString("-="),
      QString("*="),
      QString("/="),
      QString("~="),
      QString("+"),
      QString("-"),
      QString("*"),
      QString("/"),
      QString("&"),
      QString("|"),
      QString("~"),
      QString("^"),
      QString("<<"),
      QString(">>"),
      // Compare:
      QString("=="),
      QString("<"),
      QString(">"),
      QString("<="),
      QString(">="),
      // Syntax:
      QString("#"),   // length
      QString("("),
      QString(")"),
      QString("["),
      QString("]"),
      QString(","),
      QString("."),   // member access
      QString(";"),
      QString(":"),   // selfcall
      QString(".."),  // concatenate
      QString("..."), // pack
   };

   static bool is_keyword_boundary(QChar c) {
      if (c == '_')
         return false;
      if (c.isLetterOrNumber())
         return false;
      return true;
   }

   static bool is_name_char(QChar c) {
      // Per Lua 5.4 manual section 3.1: names can be Latin letters, Arabic-indic digits, and underscores, 
      // not beginning with a digit and not being a reserved word
      if (c == '_')
         return true;
      if (c.isLetterOrNumber())
         return true;
      return false;
   }

   //
   // Given a Lua long bracket, e.g. `[[` or `[===[`, extracts the level (number of equal signs) 
   // and the total length of the token. If there is no long bracket, the level is set to -1.
   // 
   // The first character in the QStringRef should be the first `[`.
   //
   static void extract_long_bracket(const QStringRef& view, int& level, int& token_length) {
      level        = -1;
      token_length =  0;
      if (view[0] != '[')
         return;
      bool match = true;
      bool ended = false;
      auto size  = view.size();
      int  i     = 1;
      for (; i < size; ++i) {
         QChar c = view[i];
         if (c != '=') {
            ended = c == '[';
            match = ended;
            break;
         }
      }
      if (match && ended) {
         token_length = i + 1;
         level        = i - 1;
      }
   }

   static bool extract_long_bracket_close(const QStringRef& view, int level, int& token_length) {
      token_length = 0;
      if (view[0] != ']')
         return false;
      bool match = true;
      bool ended = false;
      auto size  = view.size();
      int  i     = 1;
      for (; i < size; ++i) {
         QChar c = view[i];
         if (c != '=') {
            ended = c == ']';
            match = ended;
            break;
         }
      }
      if (match && ended) {
         if (i - 1 == level) {
            token_length = i + 1;
            return true;
         }
      }
      return false;
   }

   static const QString* extract_keyword(const QStringRef& view) {
      auto size = view.size();
      for (auto& keyword : keywords) {
         if (view.startsWith(keyword)) {
            auto kw_size = keyword.size();
            bool bounded = true;
            if (size > kw_size)
               bounded = is_keyword_boundary(view[kw_size]);
            if (bounded)
               return &keyword;
         }
      }
      return nullptr;
   }

   static const QString* extract_operator(const QStringRef& view) {
      for (auto& keyword : operators)
         if (view.startsWith(keyword))
            return &keyword;
      return nullptr;
   }
}

DKLuaSyntaxHighlighter::DKLuaSyntaxHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {
   this->formats.comment.block.setFontItalic(true);
   this->formats.comment.block.setForeground(QColor::fromRgb(48, 192, 8));
   this->formats.comment.line.setFontItalic(true);
   this->formats.comment.line.setForeground(QColor::fromRgb(48, 192, 8));
   this->formats.keyword.setFontWeight(QFont::Weight::Bold);
   this->formats.keyword.setForeground(QColor::fromRgb(0, 16, 255));
   this->formats.label.setFontWeight(QFont::Weight::Bold);
   this->formats.label.setForeground(QColor::fromRgb(128, 128, 0));
   this->formats.number.setForeground(QColor::fromRgb(200, 100, 0));
   this->formats.op.setForeground(QColor::fromRgb(0, 0, 128));
   this->formats.op.setFontWeight(QFont::Weight::Bold);
   this->formats.string.simple.setForeground(QColor::fromRgb(140, 140, 140));
   this->formats.string.block.setForeground(QColor::fromRgb(160, 0, 80));
}
void DKLuaSyntaxHighlighter::highlightBlock(const QString& text) {
   auto state   = BlockState(this->previousBlockState());
   auto initial = state;
   int  start   = 0;
   const auto size = text.size();
   //
   if (state.is_none()) {
      //
      // Catch keywords at the start of the block:
      //
      for (auto& keyword : keywords) {
         if (text.startsWith(keyword)) {
            auto kw_size = keyword.size();
            if (size > kw_size) {
               if (!is_keyword_boundary(text[kw_size]))
                  continue;
            }
            this->setFormat(0, kw_size, this->formats.keyword);
            break;
         }
      }
   }
   //
   bool backslash = false;
   for (int i = 0; i < size; ++i) {
      QChar c  = text[i];
      auto  cd = text.mid(i, 2);
      if (state.is_none()) {
         if (c == '"' || c == '\'') {
            start = i;
            state = BlockState(TokenType::String_Simple, c.toLatin1());
            continue;
         }
         if (c == '[') {
            int length =  0;
            int level  = -1;
            extract_long_bracket(QStringRef(&text, i, size - i), level, length);
            if (level > BlockState::max_supported_param)
               level = -1;
            if (level >= 0) {
               start = i;
               state = BlockState(TokenType::String_Block, level);
               continue;
            }
         }
         if (cd == "--") {
            //
            // Handle comments:
            //
            int   length =  0;
            int   level  = -1;
            QChar e      = (i + 2) < size ? text.at(i + 2) : '\0';
            if (e == '[') {
               extract_long_bracket(QStringRef(&text, i + 2, size - i - 2), level, length);
               if (level > BlockState::max_supported_param)
                  level = -1;
            }
            start = i;
            if (level >= 0) {
               state = BlockState(TokenType::Comment_Block, level);
            } else {
               state = BlockState(TokenType::Comment_Line);
            }
            i += length;
            continue;
         }
         if (cd == "::") {
            //
            // Handle labels i.e. `::label_name::`
            //
            int  l = text.length();
            int  j = i + 2;
            bool match = true;
            bool ended = false;
            for (; j < l; ++j) {
               auto c = text[j];
               if (!is_name_char(c)) {
                  if (c == ':' && j + 1 < l && text[j + 1] == ':') { // end-delimiter
                     ended = true;
                     break;
                  }
                  match = false;
                  break;
               }
            }
            if (match && ended) {
               this->setFormat(i, j + 2 - i, this->formats.label);
               i = j + 2 - 1; // subtract 1 because we're in a for-loop, which will increment
               continue;
            }
         }
         //
         auto next = QStringRef(&text, i, size - i);
         if (i && is_keyword_boundary(text[i - 1]) && !is_keyword_boundary(c)) {
            auto  view = QStringRef(&text, i, size - i);
            auto* keyword = extract_keyword(view);
            if (keyword) {
               auto length = keyword->size();
               this->setFormat(i, length, this->formats.keyword);
               i += length - 1;
               continue;
            }
         }
         if (i == 0 || is_keyword_boundary(text[i - 1])) {
            if (c.isDigit()) {
               int j = i;
               for (; j < size; ++j) {
                  if (!text[j].isDigit())
                     break;
               }
               int length = j - i;
               this->setFormat(i, length, this->formats.number);
               i += length - 1;
               continue;
            }
         }
         if (auto* keyword = extract_operator(next)) {
            auto length = keyword->size();
            this->setFormat(i, length, this->formats.op);
            i += length - 1;
            continue;
         }
         continue;
      }
      //
      // We're in some sort of special token.
      //
      auto next   = QStringRef(&text, i, size - i);
      int  length = 0;
      switch (state.code()) {
         case TokenType::String_Simple:
            backslash = (c == '\\');
            if (c == QChar::fromLatin1(state.param())) {
               this->setFormat(start, i - start + 1, this->formats.string.simple);
               state.clear();
            }
            break;
         case TokenType::String_Block:
            if (extract_long_bracket_close(next, state.param(), length)) {
               this->setFormat(start, i - start + length, this->formats.string.block);
               state.clear();
               i += length - 1;
            }
            break;
         case TokenType::Comment_Block:
            if (extract_long_bracket_close(next, state.param(), length)) {
               if (text.mid(i + length, 2) == "--") {
                  this->setFormat(start, i - start + length + 2, this->formats.comment.block);
                  state.clear();
                  i += length + 2 - 1;
               }
            }
            break;
      }
   }
   //
   // Reached the end of the block. We typically apply formatting to tokens when we've 
   // encountered their ends, so if we're inside of any tokens now, we need to apply 
   // formatting at this time.
   // 
   // Qt's documentation doesn't say this outright, but the end of a text block is always 
   // either the end of a line or the end of the document entirely. As such, any states 
   // that should not persist past the end of the line must be exited here.
   //
   switch (state.code()) {
      case TokenType::Comment_Block:
         this->setFormat(start, text.size() - start, this->formats.comment.block);
         break;
      case TokenType::Comment_Line:
         this->setFormat(start, text.size() - start, this->formats.comment.line);
         state.clear();
         break;
      case TokenType::String_Simple:
         if (backslash || initial.code() == TokenType::String_Simple) {
            this->setFormat(start, text.size() - start, this->formats.string.simple);
         } else {
            state.clear();
         }
         break;
      case TokenType::String_Block:
         this->setFormat(start, text.size() - start, this->formats.string.block);
         break;
   }
   setCurrentBlockState(state.to_int());
}