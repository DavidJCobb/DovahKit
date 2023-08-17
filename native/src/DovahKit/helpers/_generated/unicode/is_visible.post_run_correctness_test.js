{
   let code = constexpr_function_for_test(
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

   code = code.replaceAll("std::uint32_t ", "");
   code = code.replaceAll(/constexpr bool [^\(]+/g, "function ___run_test");

   code = eval(code);
   if (!code)
      code = ___run_test;
   console.log(code);

   for(let i = 0; i < UNICODE_CHARS.length; ++i) {
      function should_be_visible(glyph) {
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
      }

      let ch = UNICODE_CHARS[i];
      
      let test = code(i);
      if (test != should_be_visible(ch)) {
         console.log("Wrong: " + i, ch, test);
         break;
      }
   }
   console.log("If nothing was logged as wrong, then we're all good!");
}