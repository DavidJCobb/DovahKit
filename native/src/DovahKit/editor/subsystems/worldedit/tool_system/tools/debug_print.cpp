#include "./debug_print.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include <QString>

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void debug_print::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      response res = { o.text };

      if (input.has_range) {
         auto x = input.range.x;
         auto y = input.range.y;

         res.text += " (";
         if (input.range.is_delta && x > 0)
            res.text += "+";
         res.text += QString::number(x).toStdString();
         res.text += ", ";
         if (input.range.is_delta && y > 0)
            res.text += "+";
         res.text += QString::number(y).toStdString();
         res.text += ")";
      }

      all_results.merge_member(input, res);
   }
   /*static*/ void debug_print::request_for_hold_release(const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      response res;
      res.text = std::string{"[Hold-Release] "} + o.text;
      all_results.merge_member(res);
   }
}

//#include "../../core.h" // not needed for this tool

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void debug_print::invoke(const response& params) {
      qDebug(params.text.c_str());
   }
}