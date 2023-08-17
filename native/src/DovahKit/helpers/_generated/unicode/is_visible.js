// for use on https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
// given info at https://www.unicode.org/L2/L1999/UnicodeData.html

//
// We could generate a constexpr list of structs representing metadata for every character
// in the Unicode standard. The problem with this is that iterating over these structs would
// probably blow *way* past the constexpr instruction limit. What's that, you ask? Compilers
// limit how many instructions they execute in a single compile-time expression in order to
// guard against infinite loops and other issues; it's a shortcut workaround to the halting
// problem.
//
// It's the number of instructions executed, not the number of instructions written. If you
// run a single-instruction loop over one million elements in a list, that's (at least) one 
// million instructions.
//

const UNICODE_CHARS = (function() {
   let chars  = [];
   let ranges = [];

   let lines = document.documentElement.innerText.split("\n");
   for(let line of lines) {
      line = line.split(";");
      if (!line[1] && line[1] !== "")
         continue;

      let code = parseInt(line[0], 16);
      
      {
         let name = line[1];
         let data = name.match(/^\<(.*), (First|Last)\>$/);
         if (data) {
            if (data[2] == "Last") {
               ranges[ranges.length - 1].max = code;
               continue;
            }
            ranges.push({
               min: code,
               data: {
                  name:          data[1],
                  category_gen:  line[2],
                  combining:     line[3],
                  category_bidi: line[4],
                  decomposition: line[5],
                  decimal_digit: line[6],
                  digit_value:   line[7],
                  numeric_value: line[8],
                  mirrored:      line[9],
                  uni_1_name:    line[10],
                  comment_10646: line[11],
                  uppercase_map: line[12],
                  lowercase_map: line[13],
                  titlecase_map: line[14],
               },
            });
            continue;
         }
      }
      
      let info = {
         name:          line[1],
         category_gen:  line[2],
         combining:     line[3],
         category_bidi: line[4],
         decomposition: line[5],
         decimal_digit: line[6],
         digit_value:   line[7],
         numeric_value: line[8],
         mirrored:      line[9],
         uni_1_name:    line[10],
         comment_10646: line[11],
         uppercase_map: line[12],
         lowercase_map: line[13],
         titlecase_map: line[14]
      };
      (["decimal_digit", "digit_value", "numeric_value"]).forEach(function(e) {
         let field = info[e];
         if (field === "")
            info[e] = null;
         else
            info[e] = +field;
      });
      (["uppercase_map", "lowercase_map", "titlecase_map"]).forEach(function(e) {
         if (info[e] === "")
            info[e] = null;
         else
            info[e] = parseInt(info[e], 16);
      });
      
      chars[code] = info;
   }
   
   for(let range of ranges) {
      for(let i = range.min; i <= range.max; ++i)
         chars[i] = range.data;
   }
   
   return chars;
})();

/*//
const UNICODE_RANGES = (function() {
   let items = [];

   let lines = document.documentElement.innerText.split("\n");
   let last  = null;
   for(let line of lines) {
      line = line.split(";");
      if (!line[1] && line[1] !== "")
         continue;
      
      let data = line[1].match(/^\<(.*), (First|Last)\>$/);
      if (!data)
         continue;
      
      if (data[2] == "First") {
         if (last)
            throw new Error("mismatched/overlapping ranges?");
         last = {
            min:  parseInt(line[0], 16),
            max:  null,
            name: data[1]
         };
         items.push(last);
         continue;
      } else if (data[2] == "Last") {
         if (!last)
            throw new Error("end with no start?");
         last.max = parseInt(line[0], 16);
         last = null;
         continue;
      }
   }
   
   return items;
})();
//*/

class RangeAndAtomSet {
   constructor(params) {
      this.ranges = [];
      this.atoms  = [];
      
      if (params && params.list && params.test) {
         let list = params.list;
         let test = params.test;
         
         let min  = params.min || 0;
         let max  = (params.max === undefined) ? list.length : params.max;
         
         for(let i = min; i < max; ++i) {
            if (!test(list[i]))
               continue;
            let consecutive = 0;
            let j = i + 1;
            for(; j < max; ++j) {
               if (test(list[j]))
                  ++consecutive;
               else
                  break;
            }
            if (consecutive > 1) {
               this.ranges.push({ min: i, max: i + consecutive });
            } else if (consecutive == 1) {
               this.atoms.push(i);
               this.atoms.push(i + 1);
            } else {
               this.atoms.push(i);
            }
            i = j; // no + 1 because the loop advancement does that for us
         }
      }
   }

   matches(n) {
      for (let range of this.ranges) {
         if (n >= range.min && n <= range.max)
            return true;
         if (n > range.max) // assume list is ordered
            break;
      }
      return this.atoms.includes(n);
   }

   // very quick and dirty
   invert(min, max) {
      let ranges = [];
      let atoms  = [];
      for (let i = min; i < max; ++i) {
         if (this.matches(n))
            continue;
         let consecutive = 0;
         let j = i + 1;
         for (; j < max; ++j) {
            if (test(list[j]))
               ++consecutive;
            else
               break;
         }
         if (consecutive > 1) {
            ranges.push({ min: i, max: i + consecutive });
         } else if (consecutive == 1) {
            atoms.push(i);
            atoms.push(i + 1);
         } else {
            atoms.push(i);
         }
         i = j; // no + 1 because the loop advancement does that for us
      }
      this.ranges = ranges;
      this.atoms  = atoms;
   }
   
   get empty() {
      return (this.ranges.length == 0 && this.atoms.length == 0);
   }
}

function constexpr_function_for_test(name, test, default_result, indent) {
   if (!indent && indent !== "")
      indent = "";
   
   let interval_list = [];
   
   const INTERVAL = 0x1000;
   for(let i = 0; i < UNICODE_CHARS.length; i += INTERVAL) {
      let count_matching = 0;
      for(let j = i; j <= i + INTERVAL; ++j)
         if (test(UNICODE_CHARS[j]))
            ++count_matching;

      // let's minimize the number of branches: if most code points in a given interval
      // are visible, then have the default case be return true and test for non-visible;
      // if most code points in a given interval are non-visible, then have the default 
      // case be return false and test for visible.
      let tests_are_positive = (count_matching <= INTERVAL / 2);
      
      let group;
      if (tests_are_positive) {
         group = new RangeAndAtomSet({
            min:  i,
            max:  i + INTERVAL,
            list: UNICODE_CHARS,
            test: function(glyph) {
               return test(glyph);
            }
         });
      } else {
         group = new RangeAndAtomSet({
            min:  i,
            max:  i + INTERVAL,
            list: UNICODE_CHARS,
            test: function(glyph) {
               return !test(glyph);
            }
         });
      }
      group.tests_are_positive = tests_are_positive;
      
      interval_list.push(group);
   }
   
   let out   = `${indent}constexpr bool ${name}(std::uint32_t c) {` + "\n";
   let first = true;
   for (let i = 0; i < interval_list.length; ++i) {
      function stringify_charcode(c) {
         let digit_count = (("" + (INTERVAL * i - 1)).length - 1); // quick-and-dirty hack
         return "0x" + c.toString(16).toUpperCase().padStart(digit_count, "0");
      }
      function stringify_bounds(min, max, max_is_inclusive) {
         let out = "(";
         if (min > 0)
            out += `c >= ${stringify_charcode(min)} && `;
         out += `c <${max_is_inclusive ? "=" : ""} ${stringify_charcode(max)})`;
         return out;
      }

      let interval = interval_list[i];
      if (interval.empty && !interval.tests_are_positive == default_result)
         //
         // All code points in this interval would return the same value as the whole 
         // function's default return value. No point in writing in a branch for this 
         // interval.
         //
         continue;
      
      if (first) {
         first = false;
         out += `${indent}   `;
      } else {
         out += " else ";
      }
      out += "if ";
      
      if (interval.empty) {
         let min = i * INTERVAL;
         let j   = i + 1;
         for(; j < interval_list.length; ++j)
            if (!interval_list[j].empty || interval_list[j].tests_are_positive != interval.tests_are_positive)
               break;
         let max = j * INTERVAL;
         
         out += `${stringify_bounds(min, max, false)} {\n`;
         out += `${indent}      return ${!interval.tests_are_positive};\n`;
         out += `${indent}   }`;
         
         i = j - 1;
         continue;
      }
      
      out += `${stringify_bounds(i * INTERVAL, (i + 1) * INTERVAL, false)} {\n`;
      for(let range of interval.ranges) {
         out += `${indent}      if ${stringify_bounds(range.min, range.max, true)}\n`;
         out += `${indent}         return ${interval.tests_are_positive};\n`;
      }
      if (interval.atoms.length) {
         out += `${indent}      switch (c) {\n`;
         for(let atom of interval.atoms) {
            out += `${indent}         case ${stringify_charcode(atom)}:\n`;
         }
         out += `${indent}            return ${interval.tests_are_positive};\n`;
         out += `${indent}      }\n`;
      }
      if (!interval.tests_are_positive != default_result)
         out += `${indent}      return ${!interval.tests_are_positive};\n`;
      out += `${indent}   }`;
   }
   out += "\n";
   out += `${indent}   return ${default_result};\n`;
   out += `${indent}}`;
   return out;
}

constexpr_function_for_test(
   "is_visible", 
   function (glyph) {
      if (!glyph)
         return false;
      if (glyph == UNICODE_CHARS[32]) // allow ASCII whitespace
         return true;
      switch (glyph.category_gen) {
         case "Mc":
         case "Zs":
         case "Zl":
         case "Zp":
         case "Cc":
         case "Cf":
         case "Cs":
         case "Co":
         case "Cn":
            return false;
      }
      return true;
   },
   false,
   "   "
);