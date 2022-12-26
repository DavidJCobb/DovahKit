# Equation-solving cheat sheet

## Terms

<dl>
   <dt>polynomial</dt>
      <dd>Any grouping of terms that are added or subtracted, where a "term" is here a group of values multiplied together, some of which may have positive-integer exponents.</dd>

   <dt>binomial</dt>
      <dd>Two terms added or subtracted.</dd>

   <dt>trinomial</dt>
      <dd>Three terms added or subtracted.</dd>

   <dt>degree [of a polynomial]</dt>
      <dd>The highest exponent on a non-constant in a term.</dd>
</dl>

## Square-related identities

### Binomials

(<var>a</var> + <var>b</var>)<sup>2</sup> = <var>a</var><sup>2</sup> + 2<var>a</var><var>b</var> + <var>b</var><sup>2</sup>  
(<var>a</var> - <var>b</var>)<sup>2</sup> = <var>a</var><sup>2</sup> - 2<var>a</var><var>b</var> + <var>b</var><sup>2</sup>

This works in reverse, too:

<var>a</var><sup>2</sup> + <var>b</var><sup>2</sup> = (<var>a</var> + <var>b</var>)<sup>2</sup> - 2<var>a</var><var>b</var>

#### Difference of two squares

<var>a</var><sup>2</sup> - <var>b</var><sup>2</sup> = (<var>a</var> + <var>b</var>)(<var>a</var> - <var>b</var>)

#### Multiplying binomials: decompose via FOIL

First, Outside; Inside Last.

(<var>a</var> + <var>b</var>)(<var>c</var> + <var>d</var>) = <var>a</var><var>c</var> + <var>a</var><var>d</var> + <var>b</var><var>c</var> + <var>b</var><var>d</var>

#### Multiplying binomials of squared terms: group squares via <dfn>Brahmagupta-Fibonacci identity</dfn>

(<var>a</var><sup>2</sup> + <var>b</var><sup>2</sup>)(<var>c</var><sup>2</sup> + <var>d</var><sup>2</sup>) = (<var>a</var><var>c</var> - <var>b</var><var>d</var>)<sup>2</sup> + (<var>a</var><var>d</var> - <var>b</var><var>c</var>)<sup>2</sup>

and

(<var>a</var><sup>2</sup> + <var>b</var><sup>2</sup>)(<var>c</var><sup>2</sup> + <var>d</var><sup>2</sup>) = (<var>a</var><var>c</var> + <var>b</var><var>d</var>)<sup>2</sup> + (<var>a</var><var>d</var> - <var>b</var><var>c</var>)<sup>2</sup>

### Trinomials

(<var>a</var> + <var>b</var> + <var>c</var>)<sup>2</sup> = <var>a</var><sup>2</sup> + <var>b</var><sup>2</sup> + <var>c</var><sup>2</sup> + 2(<var>a</var><var>b</var> + <var>b</var><var>c</var> + <var>a</var><var>c</var>)

#### Multiplying trinomials

(<var>a</var> + <var>b</var> + <var>c</var>)(<var>d</var> + <var>e</var> + <var>f</var>) = <var>a</var><var>d</var> + <var>a</var><var>e</var> + <var>a</var><var>f</var> + <var>b</var><var>d</var> + <var>b</var><var>e</var> + <var>b</var><var>f</var> + <var>c</var><var>d</var> + <var>c</var><var>e</var> + <var>c</var><var>f</var>

## Factoring polynomials

* Converts one polynomial into the multiplication of two or more polynomials
* You can "factor" a quadrinomial (four-term polynomial) into two binomials multiplied together
* Decreases the polynomial's order (all exponents decrease by one).
  * You use the "greatest common factor" for this.

**Example A:**  
5<var>x</var><sup>2</sup> + 10<var>x</var> = 5<var>x</var>(<var>x</var> + 2)

**Example B:**  
9<var>x</var><sup>5</sup> - 9<var>x</var><sup>4</sup> + 15<var>x</var><sup>3</sup> - 15<var>x</var><sup>2</sup>

Common integer factor is 3. Common x-factor is <var>x</var><sup>2</sup>. Therefore, greatest common factor is 3<var>x</var><sup>2</sup>. Ergo the above is equivalent to:

3<var>x</var><sup>2</sup>(3<var>x</var><sup>3</sup> - 3<var>x</var><sup>2</sup> + 5<var>x</var> - 5)

## Quadratic polynomials

"Quadratic" is here just jargon for "squared;" these are polynomials with degree 2. Any univariate ("only one variable") quadratic polynomial can be written as a <dfn>quadratic function</dfn>, and any such function can be written in any of three ways (these are all mathematically equivalent):

<dl>
   <dt>Standard form</dt>
      <dd><var>f</var>(<var>x</var>) = <var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var></dd>
      <dd>When written in this form, the equation can be solved using the quadratic formula.</dd>
   <dt>Factored form</dt>
      <dd><var>f</var>(<var>x</var>) = <var>a</var>(<var>x</var> - <var>r<sub>1</sub></var>)(<var>x</var> - <var>r<sub>2</sub></var>)</dd>
      <dd>In this form, <var>r<sub>1</sub></var> and <var>r<sub>2</sub></var> are the roots of the quadratic function and the solutions of the quadratic equation. You can convert from the standard form to the factored form using the quadratic formula to obtain the roots.</dd>
   <dt>Vertex form</var>
      <dd><var>f</var>(<var>x</var>) = <var>a</var>(<var>x</var> - <var>h</var>)<sup>2</sup> + <var>k</var></dd>
      <dd>In this form, <var>h</var> and <var>k</var> are the x- and y-coordinates of a vertex. You can convert from the standard form to the vertex form by <dfn>completing the square</dfn>.</dd>
</dl>

### Completing the square

<var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var> = <var>a</var>(<var>x</var> + (<var>b</var> ÷ 2<var>a</var>))<sup>2</sup> - <var>a</var>((<var>b</var> ÷ <var>a</var>)<sup>2</sup> ÷ 4) + <var>c</var>

<var>h</var> = -<var>b</var> ÷ 2<var>a</var>  
<var>k</var> = -<var>a</var>((<var>b</var> ÷ <var>a</var>)<sup>2</sup> ÷ 4) + <var>c</var>

#### Derivation

For a non-monic polynomial (i.e. one where the leading coefficient, <var>a</var>, is not 1), we want to first factor out that leading coefficient, and then apply the process for solving a monic polynomial. We begin by dividing every term in the polynomial by that coefficient:

<var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var> = <var>a</var>(<var>x</var><sup>2</sup> + (<var>b</var> ÷ <var>a</var>)<var>x</var> + (<var>c</var> ÷ <var>a</var>))

Next, we complete the square for the inner monic polynomial.

Consider monic polynomials of the form <var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var>. Recall the binomial identities from above:

(<var>a</var> + <var>b</var>)<sup>2</sup> = <var>a</var><sup>2</sup> + 2<var>a</var><var>b</var> + <var>b</var><sup>2</sup>  
(<var>a</var> - <var>b</var>)<sup>2</sup> = <var>a</var><sup>2</sup> - 2<var>a</var><var>b</var> + <var>b</var><sup>2</sup>

We can contrive a version of these identities that is similar to our monic polynomial, where the only difference is the constant term (<var>c</var>):

(<var>x</var> + 0.5<var>b</var>)<sup>2</sup> = <var>x</var><sup>2</sup> + <var>b</var><var>x</var> + 0.25<var>b</var><sup>2</sup>

Or alternatively,

(<var>x</var> + (<var>b</var> ÷ 2))<sup>2</sup></span> = <var>x</var><sup>2</sup> + <var>b</var><var>x</var> + (<var>b</var><sup>2</sup> ÷ 4)

(It took me a moment, when researching this, to remember that 0.25 = 0.5<sup>2</sup>. More obviously, when expressing it as a division rather than a multiplication, 2<sup>2</sup> = 4.)

The above identity gives rise to:

<var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var> = (<var>x</var> + <var>b</var> ÷ 2)<sup>2</sup> + <var>k</var>  
<var>k</var> = <var>c</var> - (<var>b</var><sup>2</sup> ÷ 4)

For a monic polynomial, we're done: we've completed the square and put the polynomial into vertex form. For a non-monic polynomial, we have to multiply the original coefficient back in:

<var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var> = <var>a</var>(<var>x</var><sup>2</sup> + (<var>b</var> ÷ <var>a</var>)<var>x</var> + (<var>c</var> ÷ <var>a</var>))  
<span style="visibility:hidden"><var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var></span> = <var>a</var>((<var>x</var> + (<var>b</var> ÷ 2<var>a</var>))<sup>2</sup> + <var>k</var>)  
<span style="visibility:hidden"><var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var></span> = <var>a</var>((<var>x</var> + (<var>b</var> ÷ 2<var>a</var>))<sup>2</sup> + (<var>c</var> ÷ <var>a</var>) - ((<var>b</var> ÷ <var>a</var>)<sup>2</sup> ÷ 4))  
<span style="visibility:hidden"><var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var></span> = <var>a</var>((<var>x</var> + (<var>b</var> ÷ 2<var>a</var>))<sup>2</sup> - ((<var>b</var> ÷ <var>a</var>)<sup>2</sup> ÷ 4) + (<var>c</var> ÷ <var>a</var>))  
<span style="visibility:hidden"><var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var></span> = <var>a</var>(<var>x</var> + (<var>b</var> ÷ 2<var>a</var>))<sup>2</sup> - <var>a</var>((<var>b</var> ÷ <var>a</var>)<sup>2</sup> ÷ 4) + <var>a</var>(<var>c</var> ÷ <var>a</var>)  
<span style="visibility:hidden"><var>a</var><var>x</var><sup>2</sup> + <var>b</var><var>x</var> + <var>c</var></span> = <strong><var>a</var>(<var>x</var> + (<var>b</var> ÷ 2<var>a</var>))<sup>2</sup> - <var>a</var>((<var>b</var> ÷ <var>a</var>)<sup>2</sup> ÷ 4) + <var>c</var></strong>

Testing that with the example from Wikipedia:

3<var>x</var><sup>2</sup> + 12<var>x</var> + 27 = 3(<var>x</var><sup>2</sup> + (12 ÷ 3)<var>x</var> + (27 ÷ 3))  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3((<var>x</var> + (12 ÷ 2×3))<sup>2</sup> + <var>k</var>)  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3((<var>x</var> + (12 ÷ 2×3))<sup>2</sup> + (27 ÷ 3) - ((12 ÷ 3)<sup>2</sup> ÷ 4))  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3((<var>x</var> + (12 ÷ 2×3))<sup>2</sup> - ((12 ÷ 3)<sup>2</sup> ÷ 4) + (27 ÷ 3))  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + (12 ÷ 2×3))<sup>2</sup> - 3((12 ÷ 3)<sup>2</sup> ÷ 4) + 3(27 ÷ 3)  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + (12 ÷ 2×3))<sup>2</sup> - 3((12 ÷ 3)<sup>2</sup> ÷ 4) + 27  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + (12 ÷ 6))<sup>2</sup> - 3((12 ÷ 3)<sup>2</sup> ÷ 4) + 27  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + 2)<sup>2</sup> - 3(4<sup>2</sup> ÷ 4) + 27  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + 2)<sup>2</sup> - 3×4 + 27  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + 2)<sup>2</sup> - 12 + 27  
<span style="visibility:hidden">3<var>x</var><sup>2</sup> + 12<var>x</var> + 27</span> = 3(<var>x</var> + 2)<sup>2</sup> + 15

It works!

## Quadrics

A quadric can be represented as:

<var>F</var>(<var>x</var>, <var>y</var>, <var>z</var>) = <var>A</var><var>x</var><sup>2</sup> + <var>B</var><var>y</var><sup>2</sup> + <var>C</var><var>z</var><sup>2</sup> + <var>D</var><var>x</var><var>y</var> + <var>E</var><var>x</var><var>z</var> + <var>F</var><var>y</var><var>z</var> + <var>G</var><var>x</var> + <var>H</var><var>y</var> + <var>I</var><var>z</var> + <var>J</var> = 0

This is functionally equivalent to the following representation when <var>w</var> is 1:

<var>F</var>(<var>x</var>, <var>y</var>, <var>z</var>, <var>w</var>) = <var>a</var><var>x</var><sup>2</sup> + 2<var>b</var><var>x</var><var>y</var> + 2<var>c</var><var>x</var><var>z</var> + 2<var>d</var><var>x</var><var>w</var> + <var>e</var><var>y</var><sup>2</sup> + 2<var>f</var><var>y</var><var>z</var> + 2<var>g</var><var>y</var><var>w</var> + <var>h</var><var>z</var><sup>2</sup> + 2<var>i</var><var>z</var><var>w</var> + <var>j</var><var>w</var><sup>2</sup> = 0

This in turn is equal to:

v<sup>t</sup><var>Q</var>v = 0

Given:

v = col(<var>x</var>, <var>y</var>, <var>z</var>, <var>w</var>)  
v<sup>t</sup> = row(<var>x</var>, <var>y</var>, <var>z</var>, <var>w</var>)

and <var>Q</var> = a 4x4 matrix:
<table>
   <tr>
      <td><var>a</var></td>
      <td><var>b</var></td>
      <td><var>c</var></td>
      <td><var>d</var></td>
   </tr>
   <tr>
      <td><var>b</var></td>
      <td><var>e</var></td>
      <td><var>f</var></td>
      <td><var>g</var></td>
   </tr>
   <tr>
      <td><var>c</var></td>
      <td><var>f</var></td>
      <td><var>h</var></td>
      <td><var>i</var></td>
   </tr>
   <tr>
      <td><var>d</var></td>
      <td><var>g</var></td>
      <td><var>i</var></td>
      <td><var>j</var></td>
   </tr>
</table>

To raytrace a quadric, you set v to the ray equation <var>r</var> = <var>r<sub>o</sub></var> + <var>h<sub>d</sub></var><var>r<sub>d</sub></var>:

(<var>h<sub>d</sub></var><var>r<sub>d</sub></var>)<sup>t</sup><var>Q</var>(<var>h<sub>d</sub></var><var>r<sub>d</sub></var>) = 0

You can break up the multiplications:

<var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>o</sub></var> + <var>h<sub>d</sub></var><var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var> + <var>h<sub>d</sub></var><var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var> + <var>h<sub>d</sub></var><sup>2</sup><var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var>

The two middle terms end up being identical as a result of <var>Q</var> being symmetric across its diagonal, so you can group them. If we also rearrange the terms in order of descending exponent,...

<var>h<sub>d</sub></var><sup>2</sup><var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var> + 2<var>h<sub>d</sub></var><var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var> + <var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>o</sub></var>

...then we end up with a quadratic equation centered on <var>h<sub>d</sub></var>, our hit distance. We can therefore use the quadratic formula:

<var>a</var> = <var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var>  
<var>b</var> = 2<var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var>  
<var>c</var> = <var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>o</sub></var>

<var>h<sub>d</sub></var> = <span style="display:inline-block;text-align:center;vertical-align:middle">
<span style="border-bottom:1px solid">-<var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>o</sub></var> ± √<span style="border-top:1px solid">(<var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>o</sub></var>)<sup>2</sup> - (<var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var>)(<var>r<sub>o</sub></var><sup>t</sup><var>Q</var><var>r<sub>o</sub></var>)</span></span><br/>
<var>r<sub>d</sub></var><sup>t</sup><var>Q</var><var>r<sub>d</sub></var>
</span>

...but I... don't <em>know</em> how to go any further than this.