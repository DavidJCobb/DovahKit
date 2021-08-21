#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class DKLuaSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
   public:
       DKLuaSyntaxHighlighter(QTextDocument* parent = nullptr);

       enum class TokenType {
          None,
          Comment_Line,
          Comment_Block,
          String_Simple,
          String_Block,
       };

       class BlockState {
          protected:
             int value = 0;

             static constexpr const int code_bitcount = 8;
             static constexpr const int code_mask     = (1 << code_bitcount) - 1;
             static_assert(code_bitcount < sizeof(int) * 8);

          public:
            BlockState() {}
            BlockState(int i) : value(i) {}
            BlockState(TokenType t, int param = 0) : value((int)t | (param << code_bitcount)) {}

            static constexpr const int param_bits          = sizeof(int) * 8 - code_bitcount;
            static constexpr const int max_supported_param = (1 << param_bits) - 1;

            inline TokenType code() const noexcept { return (TokenType)(this->value & code_mask); }
            inline int param() const noexcept { return this->value >> code_bitcount; }

            inline bool is_none() const noexcept { return (this->value == -1) || code() == TokenType::None; }

            inline void clear() noexcept {
               this->value = 0;
            }

            inline int to_int() const noexcept { return this->value; }
       };

   protected:
       void highlightBlock(const QString &text) override;

   private:
       struct {
          struct {
             QTextCharFormat line;
             QTextCharFormat block;
          } comment;
          QTextCharFormat keyword;
          QTextCharFormat label;
          QTextCharFormat number;
          QTextCharFormat op;
          struct {
             QTextCharFormat simple;
             QTextCharFormat block;
          } string;
       } formats;
};