#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class DKLuaSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
   public:
       DKLuaSyntaxHighlighter(QTextDocument* parent = nullptr);

       struct BlockState {
          enum {
             None = -1,
             Comment_Line,
             Comment_Block,
             String_Double,
             String_Single,
          };
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
          QTextCharFormat string;
       } formats;
};