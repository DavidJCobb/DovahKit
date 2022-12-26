#include "ray_cone_intersection.h"
#include <array>
#include <limits>
#include <intrin.h>
#include "helpers/simd/min_register.h"

namespace cobb::geometry {
   extern bool ray_cone_intersection_simd(
      const ::glm::vec4& Ro,
      const ::glm::vec4& Rd, // must be normalized
      //
      const ::glm::vec4& Ct,
      const ::glm::vec4& Cb,
      float Cr,

      const bool hits_from_inside_count,
      
      float& out_hit_distance
   ) {
      constexpr auto INFINITE_DISTANCE = std::numeric_limits<std::decay_t<decltype(out_hit_distance)>>::infinity();

      __m128 RoM = _mm_load_ps(&Ro.x);
      __m128 CtM = _mm_load_ps(&Ct.x);
      __m128 Rl  = _mm_sub_ps(RoM, CtM);
      __m128 Cs  = _mm_load_ps(&Cb.x);
      Cs = _mm_sub_ps(CtM, Cs);

      // Ch
      __m128 Ca;
      __m128 Ch;
      __m128 Cq = _mm_load1_ps(&Cr);
      {
         Ch = _mm_mul_ps(Cs, Cs);
         Ch = _mm_hadd_ps(Ch, Ch); // [  a,   b,   c,   d] -> [    a+b,     c+d,     a+b,     c+d]
         Ch = _mm_hadd_ps(Ch, Ch); // [a+b, c+d, a+b, c+d] -> [a+b+c+d, a+b+c+d, a+b+c+d, a+b+c+d]
         Ca = _mm_rsqrt_ps(Ch);
         Cq = _mm_mul_ps(Cq, Ca);
         Ca = _mm_mul_ps(Cs, Ca);
         Ch = _mm_sqrt_ss(Ch);
      }

      std::array<float, 4> quadratic_coefficients; // a, b, c, junk
      {
         //
         // We need to compute:
         // 
         //    a = Rd_dot_Rd - (Cq + 1) * (Rd_dot_Ca * Rd_dot_Ca);
         //    b = 2 * (Rd_dot_Rl - (Cq + 1) * Rl_dot_Ca * Rd_dot_Ca);
         //    c = Rl_dot_Rl - (Cq + 1) * Rl_dot_Ca * Rl_dot_Ca;
         // 
         // We can do a not insubstantial amount of our math in SIMD:
         // 
         //    [Rd_dot_Rd, Rd_dot_Rl, Rl_dot_Rl, *]
         //  - (
         //       [   Cq + 1,    Cq + 1,    Cq + 1, *]
         //     * [Rd_dot_Ca, Rl_dot_Ca, Rl_dot_Ca, *]
         //     * [Rd_dot_Ca, Rd_dot_Ca, Rl_dot_Ca, *]
         //    )
         // 
         // From there, we can do:
         // 
         //    a = v[0]
         //    b = 2 * v[1]
         //    c = v[2]
         // 
         // Thus, let's define:
         //
         //    r = [Rd_dot_Rd, Rd_dot_Rl, Rl_dot_Rl, *]
         //    s = [Rd_dot_Ca, Rl_dot_Ca, Rl_dot_Ca, *]
         //    t = [Rd_dot_Ca, Rd_dot_Ca, Rl_dot_Ca, *]
         //    u = [   Cq + 1,    Cq + 1,    Cq + 1, *]
         //
         // We can compute these as follows:
         // 
         //    r = [Rd.x*Rd.x + Rd.y*Rd.y + Rd.z*Rd.z + 0,  Rd.x*Rl.x + Rd.y*Rl.y + Rd.z*Rl.z + 0,  Rl.x*Rl.x + Rl.y+Rl.y + Rl.z*Rl.z + 0,  0 + 0 + 0 + 0]
         //    s = [Rd.x*Ca.x + Rd.y*Ca.y + Rd.z*Ca.z + 0,  Ca.x*Rl.x + Ca.y*Rl.y + Ca.z*Rl.z + 0,  Ca.x*Rl.x + Ca.y+Rl.y + Ca.z*Rl.z + 0,  0 + 0 + 0 + 0]
         //    t = [Rd.x*Ca.x + Rd.y*Ca.y + Rd.z*Ca.z + 0,  Rd.x*Ca.x + Rd.y*Ca.y + Rd.z*Ca.z + 0,  Ca.x*Rl.x + Ca.y+Rl.y + Ca.z*Rl.z + 0,  0 + 0 + 0 + 0]
         //
         // ----------------------------------------------------------------------------------------------------------------------------------------------
         // 
         //    A one-letter register name (besides R, S, T, and U) here indicates is the dot product of any two vectors prior to summation.
         //    A two-letter register name indicates the half-summations of two dot products, e.g. [a.x+a.y, a.z+0, b.x+b.y, b.z+0]
         //    A four-letter register name indicates four dot products.
         // 
         //    r = ghi_
         //    s = jkk_
         //    t = jjk_
         // 
         //    g = [Rd.x*Rd.x, Rd.y*Rd.y, Rd.z*Rd.z, 0]
         //    h = [Rd.x*Rl.x, Rd.y*Rl.y, Rd.z*Rl.z, 0]
         //    i = [Rl.x*Rl.x, Rl.y*Rl.y, Rl.z*Rl.z, 0]
         //    j = [Rd.x*Ca.x, Rd.y*Ca.y, Rd.z*Ca.z, 0]
         //    k = [Rl.x*Ca.x, Rl.y*Ca.y, Rl.z*Ca.z, 0]
         // 
         //   gh   = [  g[0]+g[1],   g[2]+g[3],   h[0]+h[1],   h[2]+h[3]]
         //   i_   = [  i[0]+i[1],   i[2]+i[3],         ...,         ...]
         //   ghi_ = [gh[0]+gh[1], gh[2]_gh[3], i_[0]+i_[1], i_[2]+i_[3]] = [g[0]+g[1]+g[2]+g[3],  h[0]+h[1]+h[2]+h[3],  i[0]+i[1]+i[2]+i[3],  ...]
         // 
         //   jk   = [  j[0]+j[1],   j[2]+j[3],   k[0]+k[1],   k[2]+k[3]]
         //   k_   = [  k[0]+k[1],   k[2]+k[3],         ...,         ...]
         //   jjk_ = [jk[0]+jk[1], jk[2]+jk[3], k_[0]+k_[1], k_[2]+k_[3]] = [j[0]+j[1]+j[2]+j[3],  k[0]+k[1]+k[2]+k[3],  k[0]+k[1]+k[2]+k[3],  ...]
         // 
         //   jj   = [  j[0]+j[1],   j[2]+j[3],   j[0]+j[1],   j[2]+j[3]]
         //   jjk_ = [jj[0]+jj[1], jj[2]+jj[3], k_[0]+k_[1], k_[2]+k_[3]] = [j[0]+j[1]+j[2]+j[3],  j[0]+j[1]+j[2]+j[3],  k[0]+k[1]+k[2]+k[3],  ...]
         // 
         //    v = r - (u * s * t)
         //
         __m128 g = _mm_load_ps(&Rd.x);
         __m128 h = _mm_mul_ps(g,  Rl);
         __m128 i = _mm_mul_ps(Rl, Rl);
         __m128 j = _mm_mul_ps(g,  Ca);
         __m128 k = _mm_mul_ps(Rl, Ca);
         g = _mm_mul_ps(g, g);
         //
         __m128 gh = _mm_hadd_ps(g, h);
         __m128 i_ = _mm_hadd_ps(i, i);
         __m128 jk = _mm_hadd_ps(j, k);
         __m128 jj = _mm_hadd_ps(j, j);
         __m128 k_ = _mm_hadd_ps(k, k);
         //
         __m128 r = _mm_hadd_ps(gh, i_);
         __m128 s = _mm_hadd_ps(jk, k_);
         __m128 t = _mm_hadd_ps(jj, k_);
         //
         __m128 u = _mm_add_ps(Cq, _mm_set1_ps(1.0));
         __m128 v = _mm_sub_ps(r, _mm_mul_ps(_mm_mul_ps(u, s), t));
         //
         // As a reminder:
         // 
         //    a = v[0]
         //    b = 2 * v[1]
         //    c = v[2]
         //
         _mm_store_ps(quadratic_coefficients.data(), v);
         quadratic_coefficients[1] *= 2.0F;
      }
      //
      ::glm::vec4 cone_axis;
      _mm_store_ps(&cone_axis.x, Ca);
      //
      std::array<float, 4> hits; // near, away, disc (if needed), junk
      hits[2] = hits[3] = INFINITE_DISTANCE; // to avoid interfering with the vector-min operation below
      auto count = quadratic_roots(quadratic_coefficients[0], quadratic_coefficients[1], quadratic_coefficients[2], hits[0], hits[1]);
      if (count == 0) {
         return false;
      }
      //
      // Now, we need to take our intersection points and ensure that they lie on the 
      // surface of a finite cone. If one of them is below the finite cone, then we need 
      // to check for a valid intersection with the cone's base.
      //
      __m128 RdM = _mm_load1_ps(&Rd.x);
      __m128 distance_near = _mm_load1_ps(&hits[0]);
      __m128 distance_away = _mm_load1_ps(&hits[1]);
      __m128 HpN = _mm_add_ps(RoM, _mm_mul_ps(RdM, distance_near));
      __m128 HpA = _mm_add_ps(RoM, _mm_mul_ps(RdM, distance_away));

      __m128 rule_out_hits; // mask: use 0xFFFFFFFF (true comparisons) to rule out distances
      {
         __m128 Ho  = _mm_mul_ps(HpN, Ca);
         Ho  = _mm_hadd_ps(Ho, Ho);
         __m128 HoA = _mm_mul_ps(HpA, Ca);
         HoA = _mm_hadd_ps(HoA, HoA);
         Ho  = _mm_hadd_ps(Ho,  HoA);

         // rule out hits behind the ray origin:
         rule_out_hits = _mm_load_ps(hits.data());
         rule_out_hits = _mm_cmplt_ps(rule_out_hits, _mm_setzero_ps()); // rule out distances if: hit distance < 0
         // rule out hits above the cone:
         rule_out_hits = _mm_or_ps(rule_out_hits, _mm_cmplt_ps(Ho, _mm_setzero_ps())); // rule out distances if: height offset < 0
         // rule out hits below the cone:
         __m128 below = _mm_cmpgt_ps(Ho, Ch); // rule out distances if: height offset > height
         rule_out_hits = _mm_or_ps(rule_out_hits, below);
      }

      int    valid_count;
      int    valid_hits;
      __m128 distances = _mm_load_ps(hits.data());
      {  // Rule out invalid distances.
         __m128 maxes = _mm_set1_ps(INFINITE_DISTANCE);
         maxes     = _mm_and_ps(maxes, rule_out_hits);
         distances = _mm_max_ps(maxes, distances);
         _mm_store_ps(hits.data(), distances);
         //
         valid_hits  = _mm_movemask_ps(_mm_cmplt_ps(distances, maxes));
         valid_count = (valid_hits & 1) + ((valid_hits & 2) >> 1);
      }
      if (valid_count == 0) {
         if (hits_from_inside_count) {
            //
            // The ray never hits the bounded cone's curved surface. If it originates 
            // from inside the cone and points "downward," however, it could still hit 
            // the cone's endcap from inside.
            //
            union vec34 {
               ::glm::vec3 v;
               std::array<float, 4> f;
            };
            vec34 CaV;
            _mm_store_ps(&CaV.v.x, Ca);
            //
            float dist;
            if (ray_disc_intersection(Ro, Rd, Cb, CaV.v, Cr, dist)) {
               out_hit_distance = dist;
               return true;
            }
         }
         return false;
      }
      if (valid_count == 1) {
         //
         // The ray hits the cone's curved surface only once. This can only happen under 
         // two cases: the ray originates from inside the cone, and points outward; or 
         // the ray passes through the bounded cone once and through the cone's endcap.
         //
         // For ray/disc, we first need ray/plane:
         //  - Rd_dot_Ca > 1e-8
         //  - Hd = (Cb - Ro) dot Ca / Rd_dot_Ca
         //  - Hd >= 0
         // 
         __m128 Rd_dot_Ca = _mm_load_ps(&Rd.x);
         __m128 CbM = _mm_load_ps(&Cb.x);
         __m128 Hd = _mm_sub_ps(CbM, RoM);
         Rd_dot_Ca = _mm_mul_ps(Rd_dot_Ca, Ca);
         Hd = _mm_mul_ps(Hd, Ca);
         Rd_dot_Ca = _mm_hadd_ps(Rd_dot_Ca, Rd_dot_Ca);
         Hd = _mm_hadd_ps(Hd, Hd);
         Rd_dot_Ca = _mm_hadd_ps(Rd_dot_Ca, Rd_dot_Ca);
         Hd = _mm_hadd_ps(Hd, Hd);
         Hd = _mm_div_ps(Hd, Rd_dot_Ca);
         //
         // Now for the ray/disc checks:
         //  - Hp = Ro + Rd*Hd
         //  - Dd = Hp - Do
         //  - Dd dot Dd <= Dr * Dr
         //
         __m128 Hp = _mm_add_ps(RoM, _mm_mul_ps(RdM, Hd));
         __m128 Dd = _mm_sub_ps(Hp, CbM);
         // Dd = Dd dot Dd:
         Dd = _mm_mul_ps(Dd, Dd);
         Dd = _mm_hadd_ps(Dd, Dd);
         Dd = _mm_hadd_ps(Dd, Dd);

         // And compare. Invert the conditions; you'll see why.
         __m128 cond_plane = _mm_cmple_ss(Rd_dot_Ca, _mm_set1_ps(1e-8));
         __m128 cond_disc  = _mm_cmpgt_ss(Dd, _mm_load1_ps(&Cr));
         __m128 cond_all   = _mm_and_ps(cond_plane, cond_disc);
         //
         // If the distance matches these SIMD conditions, it will be set to infinity.
         //
         {
            __m128 maxes = _mm_set1_ps(INFINITE_DISTANCE);
            maxes = _mm_and_ps(maxes, cond_all);
            Hd    = _mm_max_ss(maxes, Hd);
         }
         hits[2] = _mm_cvtss_f32(Hd);
         if (!hits_from_inside_count) {
            //
            // If we don't allow hits from the inside, then require that the disc hit work.
            //
            if (hits[2] == INFINITE_DISTANCE) {
               return false;
            }
         }
      }
      distances = _mm_load_ps(hits.data());
      distances = cobb::simd::min_register(distances);
      out_hit_distance = _mm_cvtss_f32(distances);
      return true;
   }
}