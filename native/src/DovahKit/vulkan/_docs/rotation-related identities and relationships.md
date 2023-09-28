
## Euler conventions and handedness

* Handedness refers to whether the space is clockwise or counterclockwise. When you are looking towards a negative axis -- when the axis is pointing towards you; when "positive" is behind you and "negative" is in front of you -- does increasing an object's rotation about that axis cause it to turn clockwise? If so, then the space is clockwise and lefthanded; otherwise, the space is counterclockwise and righthanded. This test is visually inverted when looking in the direction that the axis points (think about looking at a glass clock from behind: the hands would appear to spin in reverse).
* Intrinsic rotations mean that the object rotates about its own local axes: once you apply the first rotation, the rest are no longer about a world axis. Extrinsic rotations mean that all angles are always about the world axes. Games typically use intrinsic rotations.
* The angle in which Euler angles are applied will affect the results. Skyrim uses intrinsic XYZ for objects, but a typical camera would use intrinsic YZX.
  * Extrinsic XYZ and intrinsic ZYX are equivalent; thus, also, for all other orderings. Broadly speaking, intrinsic means "multiplication order" and extrinsic means "reverse multiplication order," so given matrices for X, Y, and Z, "extrinsic XYZ" would be Z times Y times X.
* "Active" and "passive" are the same as "intrinsic" and "extrinsic." However, some dorks use the two modifiers together. "Passive" inverts intrinsicness: "passive intrinsic" is extrinsic and "passive extrinsic" is intrinsic. This makes sense if you remember that intrinsic and extrinsic mirror each other: "passive" is "extrinsic," so "passive extrinsic" is doubly mirrored and becomes "intrinsic."

## Rotation matrices

The [MathDF matrix calculator with steps](https://mathdf.com/mat/) is a fabulous resource for double-checking a variety of matrix operations. The calculator works on up to three square matrices at a time. You can insert variables and expressions as matrix elements, e.g. `cos(x)`, and then compute matrix operations and see a result matrix that contains expressions (i.e. you can double-check composing a rotation matrix from varying Euler conventions this way).

### Multiplication

<var style="font-style:normal"><b>A</b></var><var style="font-style:normal"><b>B</b></var> &ne; <var style="font-style:normal"><b>B</b></var><var style="font-style:normal"><b>A</b></var>. However, <var style="font-style:normal"><b>A</b></var><var style="font-style:normal"><b>B</b></var><var style="font-style:normal"><b>C</b></var> = <var style="font-style:normal"><b>A</b></var>(<var style="font-style:normal"><b>B</b></var><var style="font-style:normal"><b>C</b></var>) = (<var style="font-style:normal"><b>A</b></var><var style="font-style:normal"><b>B</b></var>)<var style="font-style:normal"><b>C</b></var>.

### Inverse

If <var style="font-style:normal"><b>A</b></var> = <var style="font-style:normal"><b>B</b></var><var style="font-style:normal"><b>C</b></var>, then <var style="font-style:normal"><b>A</b></var><sup>-1</sup> = <var style="font-style:normal"><b>C</b></var><sup>-1</sup><var style="font-style:normal"><b>B</b></var><sup>-1</sup>. Each individual matrix is inverted and the matrix multiplication order is reversed. This generalizes to any number of matrix multiplications, e.g. if <var style="font-style:normal"><b>A</b></var> = <var style="font-style:normal"><b>B</b></var><var style="font-style:normal"><b>C</b></var><var style="font-style:normal"><b>D</b></var>, then <var style="font-style:normal"><b>A</b></var><sup>-1</sup> = <var style="font-style:normal"><b>D</b></var><sup>-1</sup><var style="font-style:normal"><b>C</b></var><sup>-1</sup><var style="font-style:normal"><b>B</b></var><sup>-1</sup>.

If a matrix is orthogonal &mdash; if its columns are all unit vectors are orthogonal (perpendicular) to each other &mdash; then the matrix's inverse is its transpose (commonly denoted as <var style="font-style:normal"><b>A</b></var><sup>T</sup>). You may sometimes see the term "orthonormal" used; in the context of two vectors or a matrix, it just means "they're all perpendicular;" it's the same as orthogonal and seems to exist purely as an extra layer of useless jargon.

### Specific matrices

#### Camera matrices

The camera matrix represents the position and orientation of the camera in space. The camera matrix isn't used directly during rendering; rather, the view matrix is its inverse. (You may want to be able to compute the camera matrix directly in order to perform camera-relative movements within your software, though.)

Let <var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, and <var>P<sub>z</sub></var> be the position-coordinates of the camera. Let <var>E<sub>x</sub></var>, <var>E<sub>y</sub></var>, and <var>E<sub>z</sub></var> be the Euler angles of the camera.

Let <var style="font-style:normal"><b>R</b>(<var>n</var>)</var> be the single-axis rotation matrix for a given Euler angle <var>n</var>. Let <var style="font-style:normal"><b>T</b>(<var>x</var>, <var>y</var>, <var>z</var>)</var> be the translation matrix for the position-coordinates <var>x</var>, <var>y</var>, and <var>z</var>.

The camera matrix <var style="font-style:normal"><b>C</b></var> is defined as follows:

> <var style="font-style:normal"><b>C</b></var> = <var style="font-style:normal"><b>T</b>(<var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, <var>P<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>?</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>?</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>?</sub></var>)</var>

You'll notice that in the above equation, the Euler angles are listed as question marks. This is because you have to decide on an Euler order to use!

Ordinary gameplay objects in Skyrim use intrinsic lefthanded XYZ rotations with X-right, Y-forward, and Z-up, which works out to <var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>. However, while the axis conventions are fine for a camera, this Euler order isn't the most intuitive for camera controls.

**A quick notation change:** For the rest of this section, let's use <var>E<sub>p</sub></var>, <var>E<sub>r</sub></var>, and <var>E<sub>h</sub></var> (i.e. pitch, roll, and heading/yaw) instead of (what would in Skyrim be) <var>E<sub>x</sub></var>, <var>E<sub>y</sub></var>, and <var>E<sub>z</sub></var>. We want to talk about what order we're compositing camera rotations in, so speaking in terms of roll, pitch, and yaw should be agnostic to axis conventions (i.e. if you're using Z-forward instead of Y-forward, then <var>E<sub>r</sub></var> would be the Z-rotation; you can map PRH to whatever coordinates you want instead of having to mentally remap XYZ to ZYX or something).

Anyway:

Cameras operating in first-person view also use intrinsic rotations, but they apply yaw first, and then pitch. Pitch is, after all, a rotation about the camera's local sideways vector; sideways is perpendicular to camera-forward, so which way is sideways depends on our heading. In the absence of a roll rotation, intrinsic yaw-then-pitch works out to <var style="font-style:normal"><b>R</b>(<var>E<sub>h</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>p</sub></var>)</var>.

If we decide to also support roll rotations, then we generally want to apply them before pitch. Their ordering relative to yaw depends on whether we conceptualize a roll as "leaning" (like in a first-person shooter) or as "barrel rolling" (like in a spacecraft). In the former case, we'd want <var style="font-style:normal"><b>R</b>(<var>E<sub>h</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>r</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>p</sub></var>)</var>, such that leaning only applies with respect to your heading (your body leans, but only your head pitches); but in the latter case, we'd want <var style="font-style:normal"><b>R</b>(<var>E<sub>r</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>h</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>p</sub></var>)</var>, such that roll is applied first and everything else is done with respect to the current roll.

For a spacecraft-style camera, then, the camera matrix is:

> <var style="font-style:normal"><b>C</b></var> = <var style="font-style:normal"><b>T</b>(<var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, <var>P<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>r</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>h</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>p</sub></var>)</var>

Here's a fun idea that I haven't tested for correctness: what if we want a slight lean effect on our spacecraft? For example, if you turn left, maybe the camera should lean left a bit to emphasize the motion. If we decide that the "camera lean sweeter" angle is <var>E<sub>l</sub></var>, then the matrix, I believe, would be:

> <var style="font-style:normal"><b>C</b></var> = <var style="font-style:normal"><b>T</b>(<var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, <var>P<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>r</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>h</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>p</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>l</sub></var>)</var>

The lean angle would be applied last, as a purely cosmetic effect. (If that doesn't work, try moving it before the pitch.)

##### Alternate axis conventions

Vulkan clip-space uses righthanded axes, with Z as the forward axis, X as the right axis, and Y as the down axis. If your world uses lefthanded axes, then you can use your projection matrix to negate the screen-/clip-/view-space Y-axis, though that will have the effect of turning all frontfaces into backfaces and vice versa (you'll need to invert backface culling in all of your shaders).

Once you're at lefthanded axes, view space will have +X = right, +Y = down, and +Z = forward. If that's the world convention you want to use (XZ lateral; Y vertical), then you're all set. However, some games, like Skyrim, use +X = right, +Y = forward, and +Z = up. How do we get there?

Well, use the lefthand rule: stick your index and middle fingers, and your thumb, out in perpendicular directions, and then hold your hand so that your index finger is pointing forward and your middle finger is pointing down. These are the view-space axes: your index finger is +Z, your middle finger is +Y, and your thumb is +X. Now, tilt your hand back towards you, so that your index finger is pointing straight up. With this ninety-degree tilt about your hand's X-axis, +Z now points up, +Y points forward, and +X still points right. There's your answer: when creating the view matrix, ***and only when creating the view matrix***, you add a -90deg X-axis tilt to the camera. You do this before any matrix inversion steps.

Your camera matrix would be laid out as if the desired convention is already in effect. For +X right, +Y forward, and +Z up, the camera matrix would treat Z-axis rotations as yaw, for example, and would not apply the -90deg offset to pitch/X.

Other axis conventions should be achievable with either a 90deg rotation or (if the desired convention is righthanded) negation of an axis potentially followed by a 90deg rotation, and AFAICT these transformations would be applied only when generating the view matrix; your camera matrix should assume that it already exists in the desired axis convention.

#### View matrices

**BLUF:**
* The view matrix is the inverse of the camera matrix.
* To invert a translation matrix, negate the coordinates.
* To invert a rotation matrix, flip the Euler order and negate the angles.
* To invert the camera matrix, invert the translation and rotation, and swap the order in which they are applied.

(With considerable appreciation to <a href="https://stackoverflow.com/a/22621286">https://stackoverflow.com/a/22621286</a>.)

<u>The view matrix <var style="font-style:normal"><b>V</b></var> is the inverse of the camera matrix <var style="font-style:normal"><b>C</b></var>.</u>

Let <var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, and <var>P<sub>z</sub></var> be the position-coordinates of the camera. Let <var>E<sub>x</sub></var>, <var>E<sub>y</sub></var>, and <var>E<sub>z</sub></var> be the Euler angles of the camera.

Let <var style="font-style:normal"><b>R</b>(<var>n</var>)</var> be the single-axis rotation matrix for a given Euler angle <var>n</var>. Let <var style="font-style:normal"><b>T</b>(<var>x</var>, <var>y</var>, <var>z</var>)</var> be the translation matrix for the position-coordinates <var>x</var>, <var>y</var>, and <var>z</var>.

The camera matrix <var style="font-style:normal"><b>C</b></var> is the combination of a translation matrix <var style="font-style:normal"><b>T</b>(<var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, <var>P<sub>z</sub></var>)</var> and a rotation matrix; supposing our chosen Euler order is intrinsic YZX, the latter matrix is <var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>. Therefore:

> <var style="font-style:normal"><b>C</b></var> = <var style="font-style:normal"><b>T</b>(<var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, <var>P<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>  
> <var style="font-style:normal"><b>V</b></var> = <var style="font-style:normal"><b>C</b></var><sup>-1</sup> = <span style="font-size:150%">(</span>&nbsp;<var style="font-style:normal"><b>T</b>(<var>P<sub>x</sub></var>, <var>P<sub>y</sub></var>, <var>P<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>&nbsp;<span style="font-size:150%">)</span><sup>-1</sup>

<var style="font-style:normal"><b>T</b>(<var>x</var>, <var>y</var>, <var>z</var>)</var><sup>-1</sup> = <var style="font-style:normal"><b>T</b>(-<var>x</var>, -<var>y</var>, -<var>z</var>)</var> for any <var>x</var>, <var>y</var>, and <var>z</var>. <u>To invert a translation matrix, negate the coordinates.</u> Therefore:

> <var style="font-style:normal"><b>V</b></var> = <var style="font-style:normal"><b>C</b></var><sup>-1</sup> = <span style="font-size:150%">(</span>&nbsp;<var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>&nbsp;<span style="font-size:150%">)</span><sup>-1</sup><var style="font-style:normal"><b>T</b>(-<var>P<sub>x</sub></var>, -<var>P<sub>y</sub></var>,- <var>P<sub>z</sub></var>)</var>

Recall that when a matrix <var style="font-style:normal"><b>A</b></var> is the product of matrices <var style="font-style:normal"><b>B</b></var>, <var style="font-style:normal"><b>C</b></var>, and so on, you can invert <var style="font-style:normal"><b>A</b></var> by inverting each of the component matrices and then reversing the order in which they're multiplied. We've now inverted the translation matrix, and reversed the order of the translation versus the rotation, so let's focus on inverting the rotation using the same technique:

>  <span style="font-size:150%">(</span>&nbsp;<var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>&nbsp;<span style="font-size:150%">)</span><sup>-1</sup> = <var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var><sup>-1</sup><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><sup>-1</sup><var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><sup>-1</sup>

Each of the individual rotation matrices that we're multiplying here represents a single-axis rotation. For all single-axis rotations, the inverse and the transpose are equivalent to sign-flips on all instances of the sine function. This is because sin(-<var>x</var>) = -sin(<var>x</var>) whereas cos(-<var>x</var>) = cos(<var>x</var>). As all single-axis rotations consist solely of either the term 1, or sine and cosine operations on the angle, this means that <var style="font-style:normal"><b>R</b>(<var>n</var>)</var><sup>-1</sup> = <var style="font-style:normal"><b>R</b>(-<var>n</var>)</var> for all Euler angles <var>n</var>. Therefore:

>  <span style="font-size:150%">(</span>&nbsp;<var style="font-style:normal"><b>R</b>(<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(<var>E<sub>z</sub></var>)</var>&nbsp;<span style="font-size:150%">)</span><sup>-1</sup> = <var style="font-style:normal"><b>R</b>(-<var>E<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(-<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(-<var>E<sub>x</sub></var>)</var>

<u>To invert a rotation matrix, flip the Euler order and negate the angles.</u> The inverse of extrinsic XYZ can be said to be "extrinsic -Z-Y-X."

Plugging that back into the view matrix equation:

> <var style="font-style:normal"><b>V</b></var> = <var style="font-style:normal"><b>C</b></var><sup>-1</sup> = <var style="font-style:normal"><b>R</b>(-<var>E<sub>z</sub></var>)</var><var style="font-style:normal"><b>R</b>(-<var>E<sub>y</sub></var>)</var><var style="font-style:normal"><b>R</b>(-<var>E<sub>x</sub></var>)</var><var style="font-style:normal"><b>T</b>(-<var>P<sub>x</sub></var>, -<var>P<sub>y</sub></var>,- <var>P<sub>z</sub></var>)</var>

<u>To invert the camera matrix, invert the translation and rotation, and swap the order in which they are applied.</u>