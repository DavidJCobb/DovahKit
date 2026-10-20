#pragma once

namespace lu {
   template<
      typename A,
      typename B = void,
      typename C = void,
      typename D = void,
      typename E = void,
      typename F = void,
      typename G = void,
      typename H = void,
      typename I = void,
      typename J = void
   >
   class passkey {
      friend A;
      friend B;
      friend C;
      friend D;
      friend E;
      friend F;
      friend G;
      friend H;
      friend I;
      friend J;
      private:
         constexpr passkey() {}
   };
}
