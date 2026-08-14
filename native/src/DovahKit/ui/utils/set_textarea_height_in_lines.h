#pragma once
class QPlainTextEdit;
class QTextEdit;

namespace ui {
   extern void set_textarea_height_in_lines(QPlainTextEdit&, size_t);
   extern void set_textarea_height_in_lines(QTextEdit&, size_t);
}