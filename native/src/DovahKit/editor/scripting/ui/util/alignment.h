#pragma once
#include <Qt>
#include <string>

namespace editor_script::util::ui {
   extern void alignment_to_string(Qt::Alignment, std::string& out);
   extern Qt::Alignment alignment_from_string(const char*, std::string& h, std::string& v, bool& h_recognized, bool& v_recognized);
}