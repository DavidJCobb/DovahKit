
# Region areas

## Line intersection

[This web page](https://web.archive.org/web/20150111083426/http://geomalgorithms.com/a05-_intersect-1.html) does a good job of explaining and showing how to test for line intersections efficiently, but it's also meant for people who actually like math. I'll provide a variation on the illustrations they use.

Let's start with a basic problem statement: we want to test for an intersection between some line segment <var><b>u</b></var> and some line segment <var><b>v</b></var>.

We can imagine that these line segments are part of lines <var>P</var> and <var>Q</var> that extend off into infinity. We can thus refer to the start and end points of <var><b>u</b></var> as <var>P<sub>0</sub></var> and <var>P<sub>1</sub></var>; similarly, the start and end points of <var><b>v</b></var> are <var>Q<sub>0</sub></var> and <var>Q<sub>1</sub></var>.

<svg>
   <g fill="currentColor">
      <circle cx="142" cy="35" r="5" />
      <circle cx="142" cy="107" r="5" />
      <circle cx="99" cy="132" r="5" />
      <circle cx="35" cy="115" r="5" />
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(137 94)" />
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(84 133) rotate(42) translate(-3 -9)" />
      <g style="font-weight: bold" stroke="none" fill="currentColor">
         <text x="55" y="143">u</text>
         <text x="150" y="75">v</text>
      </g>
      <text x="35" y="100">P₀</text>
      <text x="90" y="115">P₁</text>
      <text x="115" y="35">Q₀</text>
      <text x="115" y="107">Q₁</text>
   </g>
   <g fill="none" stroke="currentColor">
      <circle cx="142" cy="143" r="5" />
      <line x1="142" x2="142" y1="0" y2="200" />
      <line x1="0" x2="283" y1="106" y2="178" />
      <g stroke-width="2">
         <line x1="35" y1="115" x2="99" y2="132" /> <!-- u -->
         <line x1="142" y1="35" x2="142" y2="107" /> <!-- v -->
      </g>
   </g>
</svg>

Of course, lines can also be defined parametrically, such that <var>P</var>(<var>s</var>) refers to a point <var>s</var> distance along <var><b>u</b></var> (which may extend past the segment's end). Similarly, <var>Q</var>(<var>t</var>) refers to a point <var>t</var> distance along <var><b>v</b></var>. (So if <var>s</var> is equal to the length of <var><b>u</b></var>, for example, then <var>P</var>(<var>s</var>) = <var>P<sub>1</sub></var>.)

The intersection between <var>P</var> and <var>Q</var>, then, is <var>P</var>(<var>s<sub>I</sub></var>) and also <var>Q</var>(<var>t<sub>I</sub></var>).

Let's define one last vector: <var><b>w</b></var> = <var>Q<sub>0</sub></var> - <var>P<sub>0</sub></var>. In other words, this connects the starting points of <var><b>u</b></var> and <var><b>v</b></var>, pointing from the latter to the former, forming a triangle.

<svg>
   <g fill="currentColor">
      <circle cx="142" cy="35" r="5" />
      <circle cx="142" cy="107" r="5" />
      <circle cx="99" cy="132" r="5" />
      <circle cx="35" cy="115" r="5" />
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(137 94)" />
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(84 133) rotate(42) translate(-3 -9)" />
      <g style="font-weight: bold" stroke="none" fill="currentColor">
         <text x="55" y="143">u</text>
         <text x="150" y="75">v</text>
         <text x="70" y="70">w</text>
      </g>
      <text x="35" y="100">P₀</text>
      <text x="90" y="115">P₁</text>
      <text x="115" y="35">Q₀</text>
      <text x="115" y="107">Q₁</text>
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(6 -11) translate(35 115) rotate(50)" />
   </g>
   <g fill="none" stroke="currentColor">
      <circle cx="142" cy="143" r="5" />
      <line x1="142" x2="142" y1="0" y2="200" />
      <line x1="0" x2="283" y1="106" y2="178" />
      <g stroke-width="2">
         <line x1="35" y1="115" x2="99" y2="132" /> <!-- u -->
         <line x1="142" y1="35" x2="142" y2="107" /> <!-- v -->
         <line x1="142" y1="35" x2="35" y2="115" /> <!-- w -->
      </g>
   </g>
</svg>

Now, with that established, there is one more operator we need to introduce: the perpendicular dot product, or `perp_dot(a, b)`, of any two vectors <var><b>a</b></var> and <var><b>b</b></var>. This is the closest 2D equivalent to the cross product, defined as <var><b>a</b><sub>x</sub></var><var><b>b</b><sub>y</sub></var> - <var><b>b</b><sub>x</sub></var><var><b>a</b><sub>y</sub></var>. It's the dot product of the vector perpendicular to <var><b>a</b></var>, and <var><b>b</b></var>.

Note that `perp_dot(a, b)` = `-perp_dot(b, a)` = `perp_dot(-a, b)` = `perp_dot(a, -b)`.

The formula for <var>s<sub>I</sub></var> is <code>-perp_dot(v, w) / perp_dot(v, u)</code>, and the formula for <var>t<sub>I</sub></var> is <code>perp_dot(u, w) / perp_dot(u, v)</code>. If both of these values fall within the range [0, 1] then the intersection lies within the segments' bounds.

We can rephrase this in terms of any four points <var><b>a</b></var>, <var><b>b</b></var>, <var><b>c</b></var>, and <var><b>d</b></var>, treating the aforementioned <var><b>w</b></var> = <var><b>a</b></var> - <var><b>c</b></var>.

<svg>
   <g fill="currentColor">
      <circle cx="142" cy="35" r="5" />
      <circle cx="142" cy="107" r="5" />
      <circle cx="99" cy="132" r="5" />
      <circle cx="35" cy="115" r="5" />
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(137 94)" />
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(84 133) rotate(42) translate(-3 -9)" />
      <g style="font-weight: bold" stroke="none" fill="currentColor">
         <text x="55" y="143">u</text>
         <text x="150" y="75">v</text>
         <text x="70" y="70">w</text>
      </g>
      <text x="30" y="105">a</text>
      <text x="95" y="120">b</text>
      <text x="120" y="35">c</text>
      <text x="120" y="107">d</text>
      <path d="M0,0 l10,0 l-5,8 z" transform="translate(6 -11) translate(35 115) rotate(50)" />
   </g>
   <g fill="none" stroke="currentColor">
      <circle cx="142" cy="143" r="5" />
      <line x1="142" x2="142" y1="0" y2="200" />
      <line x1="0" x2="283" y1="106" y2="178" />
      <g stroke-width="2">
         <line x1="35" y1="115" x2="99" y2="132" /> <!-- u -->
         <line x1="142" y1="35" x2="142" y2="107" /> <!-- v -->
         <line x1="142" y1="35" x2="35" y2="115" /> <!-- w -->
      </g>
   </g>
</svg>

Thus:

* <var>s<sub>I</sub></var> = <code>perp_dot(c - d, a - c) / perp_dot(d - c, b - a)</code>
* <var>t<sub>I</sub></var> = <code>perp_dot(b - a, a - c) / perp_dot(b - a, d - c)</code>

We can simplify things further if we define some intermediate variables:

* <var>m</var> = <code>perp_dot(c - d, b - a)</code>
* <var>n</var> = <code>perp_dot(b - a, a - c)</code>
* <var>o</var> = <code>perp_dot(a - c, c - d)</code>

Thus:

* <var>s<sub>I</sub></var> = -<var>o</var> / -<var>m</var> = <var>o</var> / <var>m</var>
* <var>t<sub>I</sub></var> = <var>n</var> / -(-<var>m</var>) = <var>n</var> / <var>m</var>

Therefore:

* If <var>m</var> = 0, then <var><b>CD</b></var> and <var><b>BA</b></var> are collinear, which for two polygon edges should be treated as self-intersection.
* If sign(<var>m</var>) != sign(<var>o</var>), then <var>s<sub>I</sub></var> < 0 and there is no intersection.
* If abs(<var>o</var>) > abs(<var>m</var>), then <var>s<sub>I</sub></var> > 1 and there is no intersection.
* If sign(<var>m</var>) != sign(<var>n</var>), then <var>t<sub>I</sub></var> < 0 and there is no intersection.
* If abs(<var>n</var>) > abs(<var>m</var>), then <var>t<sub>I</sub></var> > 1 and there is no intersection.

These latter few conditions can be checked pretty efficiently:

```c++
if (m == 0)
   return true;
if (m > 0) {
   if (o < 0 || o > m)
      return false;
   if (n < 0 || n > m)
      return false;
} else {
   // m is negative
   if (o > 0 || o < m)
      return false;
   if (n > 0 || n < m)
      return false;
}
return true;
```