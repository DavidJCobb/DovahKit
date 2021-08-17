#include "qt_alignment.h"

namespace dovahscript::api_helpers {
   extern void alignment_to_string(Qt::Alignment align, std::string& result) {
      result.clear();
      if (align & Qt::AlignJustify)
         result = "justify";
      else if (align & Qt::AlignHCenter)
         result = "center";
      else if (align & Qt::AlignAbsolute) {
         if (align & Qt::AlignLeft)
            result = "left";
         else if (align & Qt::AlignRight)
            result = "right";
      } else {
         if (align & Qt::AlignLeft)
            result = "start";
         else if (align & Qt::AlignRight)
            result = "end";
      }
      if (result.empty())
         result = "unchanged";
      //
      result += ' ';
      //
      if (align & Qt::AlignTop)
         result += "top";
      else if (align & Qt::AlignBottom)
         result += "bottom";
      else if (align & Qt::AlignVCenter)
         result += "center";
      else if (align & Qt::AlignBaseline)
         result += "baseline";
      else
         result += "unchanged";
   }
   extern Qt::Alignment alignment_from_string(const char* value, std::string& h, std::string& v, bool& h_recognized, bool& v_recognized) {
      Qt::Alignment align;
      h.clear();
      v.clear();
      h_recognized = true;
      v_recognized = true;
      //
      {
         bool   second = false;
         size_t i = 0;
         char   c = value[i];
         do {
            if (!c)
               break;
            if (c == ' ') {
               if (second)
                  break;
               if (!h.empty())
                  second = true;
               continue;
            }
            c = tolower(c);
            if (second)
               v += c;
            else
               h += c;
         } while (c = value[++i]);
      }
      if (h == "left")
         align |= Qt::AlignLeft | Qt::AlignAbsolute;
      else if (h == "right")
         align |= Qt::AlignRight | Qt::AlignAbsolute;
      else if (h == "start")
         align |= Qt::AlignLeft;
      else if (h == "end")
         align |= Qt::AlignRight;
      else if (h == "justify")
         align |= Qt::AlignJustify;
      else if (h == "center")
         align |= Qt::AlignHCenter;
      else if (!h.empty() && h != "unchanged")
         h_recognized = false;
      //
      if (v == "top")
         align |= Qt::AlignTop;
      else if (v == "bottom")
         align |= Qt::AlignBottom;
      else if (v == "baseline")
         align |= Qt::AlignBaseline;
      else if (v == "center")
         align |= Qt::AlignVCenter;
      else if (!v.empty() && v != "unchanged")
         v_recognized = false;
      //
      return align;
   }
}