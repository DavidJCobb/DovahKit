
void alpha_testing_conditional_discard(float alpha, int operation, float threshold) {
   switch (operation) {
      case 0: // GL_ALWAYS
         break;
      case 1: // GL_LESS
         if (!(alpha < threshold))
            discard;
         break;
      case 2: // GL_EQUAL
         if (!(alpha == threshold))
            discard;
         break;
      case 3: // GL_LEQUAL
         if (!(alpha <= threshold))
            discard;
         break;
      case 4: // GL_GREATER
         if (!(alpha > threshold))
            discard;
         break;
      case 5: // GL_NOTEQUAL
         if (!(alpha != threshold))
            discard;
         break;
      case 6: // GL_GEQUAL
         if (!(alpha >= threshold))
            discard;
         break;
      case 7: // GL_NEVER
         discard;
   }
}