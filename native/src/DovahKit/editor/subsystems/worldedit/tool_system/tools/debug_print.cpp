#include "./debug_print.h"
#include "../options_union.h"
#include "../tool_results_tuple.h"

#include <QString>

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void debug_print::invoke(const tool_invocation_cause& input, const opaque_options_union& raw_options, tool_results_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      results res = { o.text };

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
   /*static*/ void debug_print::invoke_for_hold_release(const opaque_options_union& raw_options, tool_results_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      results res;
      res.text = std::string{"[Hold-Release] "} + o.text;
      all_results.merge_member(res);
   }
}