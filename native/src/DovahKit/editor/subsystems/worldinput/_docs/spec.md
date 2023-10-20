
# Worldinput

Worldinput is the system that DovahKit uses to handle user inputs within the Render Window. This system supports the creation of powerful, flexible, and complex control schemes with a wide variety of button combination behaviors. The goal of the system was to allow wildly different control schemes to be implemented declaratively and customized by the user, with the control schemes planned for launch including:

* For the keyboard and mouse:
  * **The Creation Kit controls.** Within the Creation Kit, these are not entirely defined through a standard system, but rather are a mix of Win32 accelerator keys, mappings between individual keys and editor functions, and bespoke handlers for holding multiple keys simultaneously.
* For an Xbox controller:
  * **Halo: Reach's Forge controls.** These controls allow the player to act as a flycam and manipulate one object at a time, with the camera and the held object moving in tandem. Very few button combinations are used; only basic object manipulation is mapped to single buttons, with detailed editing shunted into menus.
  * **Halo 5 and Halo Infinite's Forge controls.** These controls are more advanced; they decouple camera and selection movement in order to enable multi-selection, and they rely much more heavily on button combinations, particularly using the controller's triggers.

Some additional features were also desired, such as the ability to distinguish between short and long button presses such that different actions can be mapped to each (a feature seen in Elder Scrolls Online).

It's not possible to implement all of these control schemes in a system that maps hardcoded actions to single keys, and none of the systems use entirely the same logic for button combinations. As such, Worldinput supports defining control schemes as a node tree, wherein actions are bound to editing functions, called "tools," and can supply parameters to these tools. For example, when editing terrain in the Creation Kit, the number keys set the size of your current brush; in DovahKit, Worldedit is planned to have a single "modify brush size" tool, and each key will supply different options to that tool.

Unfortunately, this means that Worldinput is an incredibly complex system: it has to support arbitrary inputs and input combinations, while handling conflicting binds in a manner that is reasonably intuitive, and human intuition is always surprisingly complex. Some conflicts have simple solutions, e.g. conflicts like "Hold A" and "Hold A + B:" if both keys are held, we resolve the conflict in favor of the more specific bind. However, more complex conflicts arise as a result of both the intended feature set and the varying needs of the control schemes we aim to support.

# Definitions

## Bind list
A **bind list** is a flat list of keybinds generated from a bind tree. Bind trees are hierarchical arrangements of nodes which define a control scheme; bind trees get "baked" into bind lists consisting solely of fully qualified button combinations. Input handling is then done using the bind lists. As such, the contents of a bind list mirror those of a bind tree.

### Bind list item
A bind list item has the following properties:

* **Editor mode:** An optional enumeration indicating the Worldedit editor modes in which this bind is active. If the property is not set, then the bind is active in all modes.
* **Input sequence:** The input sequence that the user must enter in order to activate this bind.
* **Button press type:** The button press type associated with the input sequence: does the user have to press, long press, or hold the inputs?
* **Bound tool and associated options:** The Worldedit editing tool or function to make a request of when the bind list item activates, along with the parameters to use when making that request.

#### Persistent run-time-only state
Bind list items store the following values:

* **Press-blocked-Hold:** For a bind list item whose button press type is Hold, this flag indicates that the bind list item has been blocked by the Press-blocks-Hold behavior. It is cleared whenever the bind list item is not eligible (i.e. when its input sequence does not have a down frame status).
* **Range conflict state:** A set of flags, one for the X-axis and one for the Y-axis, which indicate whether the bind has lost a [range control conflict](#range-control-conflicts) for that axis. If so, the axis is "blocked" for the bind. This state is cleared before range conflicts are checked for, and then updated for the current frame.

### Relationships to bind tree node types
All bind list items are generated from bound tool nodes.

A bind list item's editor mode is generated from its ancestor editor mode node; if it has more than one editor mode node as an ancestor, and if these nodes specify inconsistent modes, then no bind list item is generated (because the bind would be impossible to activate).

A bind list item's input sequence is the input sequence of the original bound tool node, merged with the input sequences of any ancestor modifier nodes.

## Bind tree
A **bind tree** is a node tree wherein nodes are of the following types and subtypes:
* Root node
* Editor mode node
* Input node
   * Modifier node
   * Bound tool node

### Input node

Input nodes have input sequences attached to them. Bound tool nodes additionally have a button press type, while modifier nodes implicitly use the Hold button press type.
#### Subtypes

##### Bound tool node
A bound tool node has both a bound input, an associated tool, and a set of tool parameters. A tool is any function that can be activated by the user. Tools include:

* Move camera
* Turn camera
* Select entity
* Delete entity
* Set camera speed state (boost/precision/normal)
* Set editor mode

Tool parameters are settings that are passed to the tool, and are specific to each tool. Scalar and vector input controls can be used to supply tool parameters.

## Button press type
Input nodes that include button input controls also define a **button press type**. The button press type applies to the bound input's terminal inputs. The following button press types are recognized:

* **Press:** The terminal inputs must be held down very briefly. The bound input activates when the terminal inputs are released.
* **Long press:** The terminal inputs must be held down for a brief period of time. The bound input activates when the terminal inputs are released.
* **Hold:** The terminal inputs can be held down for any length of time. The bound input activates continuously for every frame that the terminal inputs are down, and the bound tool (if any) receives a key-up notification when the terminal inputs are finally released.

Let the *long press time threshold* refer to the minimum amount of time for which a button must be held down in order produce a long press.

Let the *press-to-hold time threshold* refer to a short delay used to disambiguate between conflicting press and hold binds, and between conflicting long press and hold binds.

## Input control
An **input control** is a single physical element of an input device which a user can physically manipulate to interact with the program. Input controls fall under three categories:

* **Buttons**, also called **keys**, supply Boolean input. A button can be either pressed (true) or released (false). Examples of buttons include the keys on a keyboard, the buttons on a mouse, the buttons on a gamepad, and the ability to click in the sticks on a gamepad.
* **Scalars** supply a single integer as input. Examples of scalars include the left and right triggers on a gamepad, which can be pressed down to varying degrees, and which can report how far down they've been pressed (e.g. 35%).
* **Vectors** supply input along two axes. Examples of vectors include the movement of the mouse, and the joysticks on a gamepad.

Scalars and vectors can both be regarded as **range controls**.

A range control is said to be "zeroed" if it is producing zero values.

### Delta controls
There is an additional, overlapping, category as well: **delta controls**. These are controls that have no consistent point of reference:

* A typical key on the keyboard has a consistent point of reference: its initial state, which is up. The "down" state is defined in contrast to this initial state.
* A joystick on an Xbox controller has a consistent point of reference: the center position, which is (0, 0). All other positions are measured relative to this center position.
* Mouse movement is a delta vector control because it has no consistent point of reference. If you move the mouse leftward, you could be moving it from any on-screen coordinate. It is the movement of the mouse &mdash; the *delta* of its position &mdash; that is meaningful, and not the position itself.
* The mouse wheel is a delta scalar control because it has no consistent point of reference: there is no single point on the wheel that represents zero; it is the movement of the wheel that is meaningful.
* Your home may have a single light that is controlled by multiple switches, with each switch toggling the light's state. The state of an individual switch isn't meaningful; rather, it is the change in switches' states that is meaningful. This would be an example of a delta button.

We may be capable of polling the physical state of a delta control faster than the operating system can (or bothers to) update it. For example, even if the user is moving the mouse cursor without stopping and at an average speed, with a sufficiently high tick rate there will be ticks where the cursor doesn't register as having moved. In practice, this can lead to tons of false-negatives on mouse movement, such that with naïve input checking, Hold binds conditioned to mouse movement can rapidly deactivate and reactivate, and Press and Long Press binds conditioned to mouse movement fail to activate.

We solve this problem by allowing delta range controls to test as "stale." For each delta range control, the timestamp of its last change should be tracked. If a delta range control tests as zeroed, but very little time has passed since its last change, then we flag the delta range control as "stale." Delta range controls cease to be stale if they remain zeroed for long enough, or immediately upon ceasing to be zeroed. Stale controls are treated as not being zeroed when we enforce an input sequence's range constraint, but if a bind has an input sequence with a range constraint, the bind will not actually fire on any frame on which the control is stale (this is so that bound tools don't repeatedly get invoked with zero/no-op inputs).

As of this writing, when the mouse stops moving (or appears to our code to have stopped moving), it will be stale for a fifteenth of a second after its last movement. If left still for longer, it will cease to be stale (becoming zeroed) afterward.

### Device button state
The following state is stored per-button, per-device:

* **Button-down timestamp:** The timestamp at which the button was last down.
* **Is down:** A single-bit flag (initially cleared) which is set at the start of each frame on which the button is down.
* **Down status changed this frame:** A single-bit flag (initially cleared) which is set at the start of the frame on which the button first goes down, and the start of the frame on which the button is released.
* **Cross-frame conflict state:** A pair of "claim" structs used to perform input node conflict resolution across multiple frames. Two sets of values exist: one for the current frame, and one for a previous frame. When an input sequence's frame status becomes *released*, it claims all buttons among its terminal inputs; that is, each button's device button state has its "current frame" claim overwritten if and only if the input sequence's specificity is greater than or equal to the specificity already present in the "current frame." On the frame after the button is released, before any further input processing begins for that frame, the "current frame" state shall overwrite the "previous frame" state if the current frame state is more specific (more on that in a moment), and then the "current frame" state shall be cleared.
   * **Timestamp:** The timestamp at which the claim was made. If this is equal to the zero timestamp, then no claim exists.
   * **Specificity:** The [specificity](#specificity) of the input sequence that made the claim. Initialized to zero.

A button is said to be *down* if the "is down" flag is set A button is said to be *released* if, on the current frame, it is not down and its down status changed on this frame. A button is said to be *up* if its "is down" flag is not set; buttons that are *released* are also *up*.

### Raycast result
Some input binds require that a raycast be performed when a particular button is first pressed down, and that the raycast yield a particular result. In these cases, the raycast result is stored as device state, associated with the button.

A button's raycast result is cleared on the frame after the button is released. It is retained for the frame on which the button is released so that it remains available to [Press and Long Press](#button-press-type) binds.

## Input sequence
An input sequence is a series of inputs that the user must make in order to perform some action. Every input sequence is defined as a tree of *input sequence groups*, optionally accompanied by a *range constraint*. Input sequences have terminal inputs.

### Persistent run-time-only state
These values exist on an input sequence only during a single session, and are used to track input sequences' state across frames.

#### Down timestamp
The *down timestamp* is the timestamp at which an input sequence's [frame status](#frame-status) first became *down*. The timestamp's value is not reset or changed on the frame at which the input sequence's frame status becomes *released*, and its value is undefined for all subsequent frames until such time as the frame status becomes *down* again.

#### Frame status
Input sequences have a *frame status* and a *frame status change flag*. The frame status is an enumeration indicating the current state of the input sequence or input sequence group. The change flag is a bool indicating whether that enumeration's value changed on the current frame.

The possible values for the frame status are:

* **inactive:** The user has not begun to enter this part of the sequence, or has not finished entering it.
* **down:** The user has entered this part of the sequence, and its terminal inputs are all down.
* **released:** The user has entered this part of the sequence (it was *down* last frame), and has released one or more of its terminal inputs on this frame. The sequence will revert to *inactive* on the next frame.[^1]

[^1]: <p>Technically, we never specifically need to know if an input sequence or input sequence group is released. What we need to know is whether it has *activated* on the current frame, and in our current design, input sequences activate when they're released.</p> <p>Suppose, hypothetically, that we were to change the design: input sequences activate as soon as their press type is knowable. That could occur when they're released, or it could occur when the terminal inputs have been down for a long enough amount of time (which, if the only eligible bind is a Press bind, could be instantly). In that case, we'd modify the algorithm – rename the "released" frame status to "activated," and have it be used when the keys are down for the right length of time. In practice, however, only activating on keyup is easier to design and implement, and the logic is simpler which I expect will be better for end users.</p>

#### Last advancement time
This is the timestamp at which the user was last seen to be making progress toward entering the full input sequence. It is updated during the input sequence update algorithm whenever:

* Any descendant ISG has a [frame status](#frame-status) of *down*.
* Any descendant ISG has a frame status of *released*.

This value is used to enforce a timeout on entering a *separate and ordered* button combination. If the user waits too long between inputs, then the button combination is interrupted and the user's "progress" within the button combination is reset.
This value must be stored on the input sequence as a whole, and not on *separate and ordered* ISGs, in order to ensure that the user is given enough time to enter all of the keys for any ISG that is the child of a *separate and ordered* ISG. ISGs have no access to their parents (primarily for performance reasons, but it's also unnecessary) and so an inner ISG cannot update the state of an outer ISG (including a hypothetical last advancement time stored locally on separate-and-ordered ISGs); the outer ISG only knows when its direct children are considered [fully] *down* and *released*.

#### Raycast success flag
This flag is set if the input sequence has a [raycast constraint](#raycast-constraint), and if that constraint has been satisfied.

The raycast constraint is first checked on the frame at which the input sequence's raycast-associated button goes down; if the constraint is met at that time, this flag is set. The flag allows us to remember that the raycast constraint was met, specifically, at the time that the raycast-associated button went down.

When that button ceases to be down, this flag is cleared.

### Input sequence group
There are four kinds of input sequence groups:

* *Single input controls*, which consist solely of one [input control](#input-control). These cannot contain any child groups. They are not, technically, "groups," but are defined as a group type for the sake of simpler algorithms and implementations.
* *Concurrent and ordered groups*, wherein the inputs must be activated in the appropriate order, and all inputs must be active at the same time.
* *Concurrent and unordered groups*, wherein the inputs can be activated in any order, but must still be active at the same time.
* *Separate and ordered groups*, wherein each input must be go down and be released after the previous input has gone down and been released.

#### Terminal inputs
When the user finishes entering an input sequence such that the input sequence's [frame status](#frame-status) is *down*, the [input controls](#input-control) that the user is holding down are the input sequence's **terminal inputs**. The input sequence's frame status will change to *released* when any of these input controls are released, and whether the user has performed [a press or a long press](#button-press-type) is determined by how long the terminal inputs were collectively down.

The terminal inputs for an input sequence group are defined as follows:

* If the group is or contains only a single input control, then that input control is the group's terminal input.
* If the group is a concurrent group, then the terminal inputs of all of its elements are, all together, the terminal inputs of the group.
* If the group is a non-concurrent group, then the terminal inputs of its final element are the terminal inputs of the group.

The terminal inputs for an entire input sequence are the terminal inputs of its root group.

Examples of terminal inputs follow, using input sequence notation.

<table>
   <thead>
      <tr>
         <th>Sequence</th>
         <th>Terminal inputs</th>
         <th>Instructions</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>[(Ctrl + Shift) + Esc]</td>
         <td>Ctrl + Shift + Esc</td>
         <td>Press and hold Ctrl and Shift in any order, and keep them held; press Esc.</td>
      </tr>
      <tr>
         <td>&lt;[A + B] + [X + Y]&gt;</td>
         <td>X + Y</td>
         <td>Press and hold A and then B; release both; press and hold X and then Y. At the end of these steps, X and Y are held; bind activates when any are released.</td>
      </tr>
      <tr>
         <td>[A + B + &lt;X + Y&gt; + LB]</td>
         <td>A + B + LB</td>
         <td>Press and hold A and then B, and keep them held; press and release X; press and release Y; press and hold LB. At the end of these steps, A, B, and LB are held; bind activates when any are released.
      </tr>
      <tr>
         <td>[A + B + &lt;X + [Y + LB]&gt;]</td>
         <td>A + B + Y + LB</td>
         <td>Press and hold A and then B, and keep them held; press and release X; press and hold Y; press and hold LB. At the end of these steps, A, B, Y, and LB are held; bind activates when any are released.
      </tr>
      <tr>
         <td>&lt;A + B + X&gt;</td>
         <td>X</td>
         <td>Press and release A; press and release B; press and release X.
      </tr>
      <tr>
         <td>[A + B + X + Y]</td>
         <td>A + B + X + Y</td>
         <td>Press and hold all of A, B, X, and Y, in any order.
      </tr>
   </tbody>
</table>

### Range constraint

An input sequence can optionally require that a [range input control](#input-control) be in a particular state in order to allow activation of the sequence. These constraints have no effect on the input sequence's [frame status](#frame-status), but are checked by the [bind list update algorithm](#bind-list-update-algorithm).

Range constraints only affect conflict resolution in the case of [concurrent activation of like binds](#simultaneous-activation-of-like-binds) with different [specificity](#specificity) values, e.g. two (Long) Press binds or two Hold binds, on the same frame. This means that the following input sequences will conflict if held at the same time, with the latter sequence (being more specific) winning the conflict:

* Hold &lt;None&gt; :: Left Stick
* Hold B :: Left Stick

Range constraints are ignored by concurrent conflict resolution when two binds have the same specificity. This ensures that the following input sequences do not conflict:

* Hold B :: Left Stick
* Hold C :: Left Stick

Range constraints are also ignored by [Press-preempts-Hold](#press-preempts-hold-desc) and [Hold-blocks-Press](#simultaneous-release-of-hold-and-press-binds). This ensures that the following input sequences do not conflict:

* Hold A :: Left Stick
* Press X :: Left Stick

The decision to implement this specific behavior stems from the fact that Press-delays-Hold and Hold-blocks-Press exist to deal with conflicts arising from the same buttons being mapped to input sequences with different button press types (whereas the concurrent binds conflict resolution rules exist to deal with like press types). Range constraints are orthogonal to button press types.

Note that range constraints can target just one axis on a two-axis range control, e.g. `Hold A :: Left Stick Y`. This means that a range constraint must specify not only a range control, but also what axes to use as input: X, Y, or both.

### Raycast constraint
An input sequence can optionally require that a raycast be made and hit targets of a given type. Raycast constraints consist of two parts:

* A **raycast requirement** struct. Most options are implementation-defined (i.e. they depend on the specific things in the editor that you can aim at), but the following are always present:
   * **"Per frame" flag:** Specifies that the raycast requirements should be rechecked on every frame after the raycast-associated button goes down, and not just on the frame that the raycast-associated button (see below) goes down.
   * **"Fail on change"** flag: Specifies that the user must continue to aim at the same raycast target, or the input sequence will cease to be active.
   * **Target types:** The set of valid targets for this raycast.
       * One of the target types should be "nothing," i.e. it should be possible for a tool to require that you click on empty space, or to allow clicks on anything (including empty space) except specific target types.
* A **raycast-associated button:** a pointer to a *single input control* [group](#input-sequence-group) somewhere in the input sequence.


### Notation and examples
The following input sequence notation is defined:

* Square brackets denote concurrent-and-ordered groups.
* Parentheses denote concurrent-and-unordered groups.
* Angle brackets denote separate-and-ordered groups.
* Double colons separate the input sequence's button combo from its directional constraint, with the constraint listed last (e.g. "Hold [A + B] :: Mouse Move").

Examples of common or Creation Kit-specific input sequences include:

<table>
   <thead>
      <tr>
         <th>Sequence</th>
         <th>Function</th>
         <th>Input description</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>A</td>
         <td>Just an example</td>
         <td>The key must be pressed.</td>
      </tr>
      <tr>
         <td>[(Ctrl + Shift) + Esc]</td>
         <td>Open Task Manager</td>
         <td>
            <p>Ctrl and Shift must both be held down at the time that Esc is pressed. Ctrl and Shift may be pressed down in any order, but must both be pressed down before Esc is pressed down.</p>
            <p>This is the common paradigm for Windows accelerator keys: the modifier keys may be pressed down in any order, but must all be pressed down at the time that the single non-modifier key is pressed down.</p>
         </td>
      </tr>
      <tr>
         <td>[Ctrl + S]</td>
         <td>Save</td>
         <td>Ctrl must be held down at the time that S is pressed.</td>
      </tr>
      <tr>
         <td>[S + LMB Drag on Selected Ref]</td>
         <td>Scale selected refs in Creation Kit</td>
      </tr>
      <tr>
         <td>[(Ctrl + Alt + S) + LMB Drag on Selected Ref]</td>
         <td>The "modify light intensity/fade" keybind in the Creation Kit</td>
         <td>Ctrl, Alt, and S must all be held down at the same time as the left mouse button for the bind to activate, but the keys may be pressed down in any order.</td>
      </tr>
      <tr>
         <td>&lt;Up + Up + Down + Down + Left + Right + Left + Right + B + A + Start&gt;</td>
         <td>Konami code.</td>
         <td>Each key must be pressed and released in order.</td>
      </tr>
   </tbody>
</table>

### Final ISG
The **final input sequence group**, or **final ISG**, in an input sequence, is the last input sequence group that the user must press down in order for the sequence as a whole to go *down*. The final ISG is principally relevant for [stitching together](#merging-of-a-parent-and-child-input-sequence) a "parent" input sequence (such as a modifier key) and a "child" input sequence into one "absolute" input sequence.

The algorithm for determining the final ISG and its parent group, given an input sequence group <var>Current</var>, is as follows:

1. Set the result variables <var>FinalGroup</var> and <var>FinalGroupParent</var> to null pointers.
2. If <var>Current</var>'s type is *single input control* or *concurrent and unordered*, then set <var>FinalGroup</var> to <var>Current</var>, and return.
3. If <var>Current</var>'s type is *concurrent and ordered* or *separate and ordered*, then:
  1. If <var>Current</var> has no child groups, then do not return a result.
  1. Let <var>LastChild</var> be the last child group of <var>Current</var>.
  1. Let <var>Nested</var> be the result of recursively running this algorithm on <var>LastChild</var>.
  1. Set <var>FinalGroup</var> to <var>Nested</var>.
  1. If <var>Nested</var> is equal to <var>LastChild</var>, then set <var>FinalGroupParent</var> to <var>Current</var>.
  1. Return.

### Specificity
An input sequence's specificity is, in simplistic terms, its length. More specifically, it is: the number of [input controls](#input-control) that the user must activate in order to change the input sequence's [frame status](#frame-status) to *down*, measured as the number of input sequence groups within the input sequence that are single controls; plus 0.5 if the input sequence has a [range constraint](#range-constraint).

Specificity is an opaque value: the precise values are not meaningful; values are only useful for comparing to each other. As such, no specific representation for specificity is required. For example, one may wish to use an integer data type by storing the specificity as double the number of buttons in the input sequence, plus one if a directional constraint is present.

<span style="page-break-after: always"></span>

## Tool

A tool is any function that can be activated via an input made to the Render Window. The input system belongs to a system called Worldinput; tools belong to a separate system called Worldedit.

The basic pattern is this:

* A bind in Worldinput is invoked. That bind makes a *request* of a Worldedit tool.
* The Worldedit tool returns a *response* representing parameters for a queued action to take.
* Worldinput merges all *responses* received for each given tool, and delivers the merged responses to whatever called it (in practice, Worldedit).
* Worldedit *invokes* each tool for which a response is present, passing the response as parameters for the invocation. This actually activates the tool and performs an action in the Render Window.

## Tool request cause

A data structure that gets passed to tools when they're requested. It has the following properties:

* **"Has button" flag:** Indicates that the [input sequence](#input-sequence) which triggered this request has one or more buttons.
* **"Has range" flag:** Indicates that the input sequence which triggered this request has a [range constraint](#range-constraint).
* **Button press type:** The [button press type](#button-press-type) of the bind that triggered this request.
* **"Button is down" flag:** Indicates that this request is the result of a button currently being down (i.e. the button press type is Hold). This is redundant with the button press type, but may be used in the future.
* **"Down status changed on this frame" flag:** Primarily useful for Hold binds, to know whether the current request is the result of the bind beginning to activate or remaining active over multiple frames. For tools that toggle some setting, for example, this is what allows them to toggle once when a Hold bind goes down and toggle back when it's released, rather than toggling every single frame. 
* **Range value:** X- and Y-axis values for a range input control.
* **"Range is delta" flag:** Indicates that the range value represents a [delta](#delta-controls) rather than an absolute position.
* **Raycast result:** Present only when the tool is requested as the result of an input sequence with a [raycast constraint](#raycast-constraint). Contains the result of the raycast that allowed the input sequence to proceed.


<span style="page-break-after: always"></span>

# Behaviors

## Conflict resolution

Binds with overlapping input controls may conflict with one another. There are several different kinds of conflicts, some of which require special handling.

### Press-preempts-Hold<span id="press-preempts-hold-desc"></span>
For the purposes of this section, a bind is "down" or "released" if its input sequence has the corresponding [frame status](#frame-status).

#### Press-delays-Hold

If a Press or Long Press bind is not released and conflicts with a Hold bind while the Hold bind is down and is eligible for activation (as determined by the [bind list update algorithm](#bind-list-update-algorithm)), then activation of the Hold bind should be delayed at least until the Hold bind has been down for sufficiently long. This will ensure that the user doesn't inadvertently activate the Hold bind when attempting to merely press or long-press the relevant button(s).

Within the Hold bind, the input controls that may potentially conflict are those which are [terminal inputs](#terminal-inputs) of the Hold input sequence's [final ISG](#final-isg).

Within the Press bind, the input controls that may potentially conflict are those which must be and have been pressed in the course of inputting the Press input sequence; that is, those input controls which are currently down and which, by virtue of becoming down at the particular times that they have (relative to other controls in the same input sequence, with the nature of said relationships depending on the type of the containing input sequence groups), are contributing toward the containing input sequence's [frame status](#frame-status) potentially becoming *down* in the future. For example, given the bind "Press [X + Y]", the Y key may potentially conflict if it is currently down, if the X key is also down, and if the Y key went down after the X key went down; and given the input bind "Press &lt;A + B&gt;", the B key may potentially conflict if it is the current item in the *separate and ordered* group therein and is currently down &mdash; that is, if the B key is currently down, and it went down soon after the A key was pressed and released.

A conflict is present if there is any overlap between the potentially conflicting input controls from the Hold bind, and the potentially conflicting input controls from the Press bind. The input controls among this overlap are <dfn>mutually conflicting keys</dfn>.

##### Hold nodes with range constraints win if the ranges are active
Consider the following binds:

* Press LMB
* Hold LMB :: Mouse Move

The former bind defines an action that should occur when the user clicks, while the latter bind defines a click-and-drag operation. If the Press-delays-Hold conflict resolution kicks in here, then all drag operations will be delayed if there is also an action on the same mouse button. This generalizes to any device that has pointer-like behaviors.

For this reason, if a conflicting Hold node's input sequence has a [range constraint](#range-constraint), and that constraint is satisfied, then the Hold node automatically wins the conflict. If in that situation the Hold node is already being delayed by Press-delays-Hold conflict resolution, then the delay stops early and the Hold node is retroamended to have won the conflict.

##### Advancing past all conflicting keys indefinitely delays the Hold node
Consider the following binds:

* Press [A + B + X + Y]
* Hold B

If the user has the A and B keys pressed down (and pressed them down in that order), then B is a mutually conflicting key and so Press-delays-Hold should kick in, applying a finite delay to the Hold bind. However, if the user has the A, B, and X keys pressed down (and pressed them down in that order), then they have advanced past all mutually conflicting keys within the Press input sequence. The desired behavior for that case is that the Hold bind be delayed indefinitely, but not specifically blocked. Should the user release the X key, the user will cease to have advanced past all mutually conflicting keys, and so the Press-delays-Hold delay will become finite again and the Hold bind may potentially win the conflict. The indefinite delay ensures that the user will always have time to complete the rest of the Press or Long Press input sequence, without being interrupted by the Hold sequence having been down for long enough to overcome a finite delay.

#### Press-blocks-Hold
If a Press or Long Press bind is released and conflicts with a Hold bind while the Hold bind is down and is eligible for activation (as determined by the [bind list update algorithm](#bind-list-update-algorithm)), then activation of the Hold bind should be blocked indefinitely until such time as the Hold bind is no longer down. The purpose of this conflict resolution rule is to ensure intuitive behavior when releasing a Press bind that has multiple terminal inputs; without Press-blocks-Hold, if multiple Hold binds conflicted with different terminal inputs in a Press bind, releasing the Press bind's terminal inputs on different frames would allow some of those Hold binds (the ones whose keys are the last ones released) to activate for a short time.

Within the Hold bind, the input controls that may potentially conflict are those which are [terminal inputs](#terminal-inputs) of the Hold input sequence's [final ISG](#final-isg).

Within the Press bind, the input controls that may potentially conflict are all terminal inputs of the Press node's entire input sequence.

A conflict is present if all of the potentially conflicting input controls from the Hold bind are also potentially conflicting input controls from the Press bind (i.e. the former collection of input controls is a subset of the latter collection of input controls).

#### Rock-Paper-Scissors case handling
If a Hold bind is down for long enough to "outlast" a conflicting Press bind, then that Press bind cannot win conflicts with any other Hold bind; any wins it has (including cases where it advanced past another Hold bind) are retroamended to losses.

#### Examples
##### Basic
Given a Press X bind and a Hold X bind, pressing and holding the X key should only activate the Hold bind after a short delay, preempting the Press bind in the process. Releasing X before that delay would instead activate the Press bind.

##### Multi-part
Consider the following binds:

* Press [Y + Z]
* Hold Y
* Hold Z

If you press and hold the Y key, then the Hold bind should activate only if the key is held down for a short delay, and its activation should block subsequent activation of the Press [Y + Z] bind until the Y key is released and re-pressed.

If you press and hold the Z key, and the Y key is already down, then the Hold Z bind should activate only if the Z key is held for a short delay; otherwise, the Press [Y + Z] bind will activate when Z is released. Moreover, if Y and Z are released on different frames, then neither Hold Y nor Hold Z will activate for the latest released button, as both Hold binds will be subject to the Press-blocks-Hold rule.

##### En route
Consider the following binds:

* Press &lt;A + S + J&gt;
* Hold S

If you've already pressed and released A, then pressing the S key should delay activation of the Hold S bind for a short time, and upon activation, the Hold bind should preempt the Press bind. Releasing the S key prior to that will instead advance the Press &lt;A + S + J&gt; bind.

This example is made complicated by the fact that there is no overlap between the two binds' terminal inputs.

##### Same prefix
Consider the following binds:

* Press [B + LS]
* Hold [B + X]

Here, if you press and hold B, and then press and hold X, the Hold bind should not be delayed. They share a terminal input (the B button), but X is not the (or rather, a) "next" key in the Press bind, nor is it a key that has already been used to advance the Press bind's input sequence. This is why we use the terminal inputs of the Hold input sequence's final ISG.

<span style="page-break-after: always"></span>

### Simultaneous activation of like binds
There are several different kinds of conflicts that can occur here.

#### Identical binds rule
If <var>A</var> and <var>B</var> have identical input sequences and use the same [button press type](#button-press-type), then no conflict is present; both binds should be allowed to activate in tandem.

#### Specificity rule
If <var>A</var> has a lower [specificity](#specificity) than <var>B</var>; and if any of the input controls in <var>A</var>'s [final ISG](#final-isg) are [terminal inputs](#terminal-inputs) of <var>B</var> or are controls in any terminal inputs of <var>B</var>, or <var>A</var> and <var>B</var> have identical non-none [range constraints](#range-constraint); then a conflict is present, and <var>B</var> should win.

If <var>A</var> and <var>B</var> have equal specificity and neither of their button press types is Hold, and there is any overlap between their terminal inputs, then a conflict is present, and both binds should be considered conflict losers: neither should activate.

##### Examples
###### Specificity difference
Consider the following binds:

* Press [A + B + C + D + E]
* Press (Y + E)

If the user presses and holds Y, and then presses and releases keys A through E in sequence, then both binds would become eligible. Neither bind is a subset of the other, and neither bind's terminal inputs are a full subset of the other; there is only overlap.

###### Specificity equivalence
Consider the following binds:

* Press [LS + X]
* Press [B + X]

If the user were to press and hold both LS and B, and then press and release X, then a conflict would occur. The desired behavior would be to deem *both* binds losers and refuse to fire either. This would be consistent with how [Windows accelerator keys](#windows-accelerator-keys) behave (inasmuch as consistency is possible given the fundamentally different design that those have).

<span style="page-break-after: always"></span>

### Simultaneous release of Hold and Press binds
If a Press or Long Press bind potentially conflicts with a Hold bind, and the two are released on the same frame, then the Press or Long Press bind should not activate.

The precise operational definition of a "potential conflict" is that the two nodes have overlapping [terminal inputs](#terminal-inputs).

#### Examples
Consider the following binds:

* Hold [A + B + C + D]
* Press [X + D]

If the user presses and holds A through D, and then presses and holds X, and then releases either X or D, then the Press bind should not fire, and the Hold bind should only fire if D was the key that was released.

#### Implementation
This must be implemented in the [bind list update algorithm](#bind-list-update-algorithm). After we've found all eligible nodes, we must check for conflicts between any Press and Long Press eligible binds, and any Hold binds that were active on the previous frame and are not eligible on this frame.

<span style="page-break-after: always"></span>

### Range control conflicts
If two different binds use the same [range control](#input-control), e.g. the same joystick on an Xbox controller, and their buttons are activated at the same time, then the binds are potentially conflicting.

If both binds consume only a single axis, and they don't consume the same axis, then they are not in conflict.

If one bind consumes all axes, and the other bind consumes only a single axis, then the binds are in conflict. The conflict should be resolved by forcing the other bind to act as though it consumes only the other, still-available, axis.

If both binds consume the same single axis, or if both binds consume all axes, then the binds are in conflict. The [specificity](#specificity) of each bind should be compared: if one is less specific than the other, then the less-specific bind should be prevented from activating at all.

These conflicts are not regarded as occurring across time; they occur only within a single frame.

#### Examples
Consider the following binds:

* Hold *nothing* :: Left Stick
* Hold RT :: Left Stick Y

When the right trigger on the controller is held, and the left stick is moved, these binds conflict. The conflict should be resolved by feeding only X-axis stick movement to the former bind.

Consider the following binds:

* Hold *nothing* :: Right Stick
* Hold B :: Right Stick

When the B button on the controller is held, and the right stick is moved, these binds conflcit. The conflict should be resolved by allowing only the latter bind to activate, as its attendant button combination is more specific.

<span style="page-break-after: always"></span>

## Conflict resolution across time
It's easy to resolve conflicts within a single frame. However, some of the above conflict resolution rules require special handling in order to handle over multiple frames.

The current approach to resolving conflicts across time is the use of [claim data structures](#device-button-state) stored in [device button state](#device-button-state). Each button can have an "active" claim, made on any previous frame, and a "pending" claim, made on the current frame. When an input sequence is released, we make the following checks on all of its [terminal inputs](#terminal-inputs) before updating its [frame status](#frame-status) to released:

* Is there an active claim on the button?

* Was the active claim made by an input sequence whose [specificity](#specificity) is greater than or equal to that of the newly-released input sequence?

* Was the active claim made after the newly-released input sequence's frame status last changed to down?

If all of these conditions are met, then the newly-released input sequence is blocked from becoming *released*; its frame status instead changes directly to *inactive*. If the input sequence belongs to a bind list item whose button press type is Press or Long Press, then this prevents that bind from firing.

If any of these conditions are not met, then the input sequence's frame status becomes *released*; moreover, all terminal inputs for the input sequence have a new claim made by the input sequence: if the pending claim's specificity is lower than or equal to that of the input sequence, then the pending claim is overwritten with the input sequence's specificity and with the current time.

This system thus applies the following conflict resolution rules across time:

* [Specificity rule](#specificity-rule)
* [Hold-blocks-Press rule](#simultaneous-release-of-hold-and-press-binds)

<span style="page-break-after: always"></span>

## Raycasts
Some tools should act on the thing the user is aiming at; and some tools should only be invocable when the player is aiming at a given kind of target. Here are a few examples:

* In Halo: Reach Forge, you can aim at an object and press A on an Xbox controller to select it. In this case, a raycast is performed once when the bind goes down.
* Halo 4's Forge mode added a safety feature: to delete an object, you must aim at the object and hold the "delete" button. In this case, a raycast is performed once when the bind goes down. In DovahKit, it is desirable to fail to recheck the raycast every frame while the bind is down, and fail to activate the bind if the player aims at something else while the button is down.
* In Halo 5 and Halo Infinite Forge, you can hold LB on an Xbox controller and then sweep your reticle over several objects, toggling their selection state. In this case, a raycast is performed every frame while a Hold bind is down.

<span style="page-break-after: always"></span>

## Seamless switching between Hold binds
The Creation Kit has a small family of "light/scale" tools, which activate when the user holds one or more keys, and then left-clicks and drags on any selected ref in the Render Window. They are as follows (all keys are concurrent in each bind):

* LMB Drag
   * Drag-move the selected refs around the environment.
* S + LMB Drag
   * Continuously adjust the scaling of all selected refs: moving the cursor upward or leftward increases; downward or rightward decreases. If any of the selected refs are lights, then this bind modifies their radii instead of their scales.
* Alt + S + LMB Drag
   * Continuously adjust the FOV of all selected spotlight refs. Conventions for cursor movement are as described above.
* Ctrl + Alt + S + LMB Drag
   * Continuously adjust the fade (brightness) of all selected light refs. The conventions for cursor movement are as described above.
* Alt + B + LMB Drag
   * Continuously adjust the shadow depth bias of all selected light refs. The conventions for cursor movement are as described above.

Each of the input sequences above has the Hold press type, and their behavior is such that as long as you keep LMB held after you trigger any of these sequences, you can press or release their various keys to swap between any of them seamlessly. Consider the following inputs, assuming that LMB is initially pressed down while the cursor is over a selected ref:

<table>
   <thead>
      <tr>
         <th colSpan="2">Action</th>
         <th rowSpan="2">Active bind</th>
         <th rowSpan="2">Current inputs</th>
      </tr>
      <tr>
         <th>Key state</th>
         <th>Key</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>keydown</td>
         <td>S</td>
         <td>None</td>
         <td>S</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>LMB</td>
         <td>Scale refs</td>
         <td>S + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Alt</td>
         <td>Scale FOVs</td>
         <td>Alt + S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>Alt</td>
         <td>Scale refs</td>
         <td>S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>S</td>
         <td>Drag-move refs</td>
         <td>LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>B</td>
         <td>Drag-move refs</td>
         <td>B + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Alt</td>
         <td>Scale shadow depth bias</td>
         <td>Alt + B + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>B</td>
         <td>Drag-move refs</td>
         <td>Alt + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>S</td>
         <td>Scale FOVs</td>
         <td>Alt + S + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Ctrl</td>
         <td>Scale light fade</td>
         <td>Ctrl + Alt + S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>Alt</td>
         <td>Scale refs</td>
         <td>Ctrl + S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>LMB</td>
         <td>None</td>
         <td>Ctrl + S</td>
      </tr>
   </tbody>
</table>

You can see that we can seamlessly switch between any of the above binds. This is a behavior that we want to replicate in DovahKit's input system.

### No seamless switching
In the above table, it's worth noting one action that *isn't* triggered by the listed inputs: saving the current file. The Creation Kit, like most Windows software, maps saving to Ctrl + S. However, when we enter the (Ctrl + Alt + S + LMB) input sequence and then begin releasing keys, we don't trigger the [Ctrl + S] bind even though there is a point at which those are the only keys that are currently down. We can seamlessly switch between Hold input sequences, but not from a Hold input sequence to a Press or Long Press input sequence.

Consider the following input sequences:

<table>
   <thead>
      <tr>
         <th>Input sequence</th>
         <th>Terminal inputs</th>
         <th>Notes</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>Hold (Ctrl + Alt + S + LMB)</td>
         <td>Ctrl + Alt + S + LMB</td>
         <td>Creation Kit bind to modify selected lights' fade values</td>
      </tr>
      <tr>
         <td>Press [Alt + 1]</td>
         <td>Alt + 1</td>
      </tr>
      <tr>
         <td>Press Alt</td>
         <td>Alt</td>
      </tr>
      <tr>
         <td>Press [Ctrl + A]</td>
         <td>Ctrl + A</td>
         <td>Common "select all" accelerator key.</td>
      </tr>
      <tr>
         <td>Press [Ctrl + S]</td>
         <td>Ctrl + S</td>
         <td>Common "save" accelerator key.</td>
      </tr>
   </tbody>
</table>

Recall that an input sequence will activate when any of its [terminal inputs](#terminal-inputs) are released. Consider, then, the following series of inputs:

<table>
   <thead>
      <tr>
         <th colSpan="2">Action</th>
         <th rowSpan="2">Active bind</th>
         <th rowSpan="2">Currently-down input controls</th>
         <th rowSpan="2">Notes</th>
      </tr>
      <tr>
         <th>Key state</th>
         <th>Key</th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td>keydown</td>
         <td>S</td>
         <td></td>
         <td>S</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>LMB</td>
         <td>Scale refs</td>
         <td>S + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Alt</td>
         <td>Scale FOVs</td>
         <td>Alt + S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>Alt</td>
         <td>Scale refs</td>
         <td>S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>S</td>
         <td>Drag-move refs</td>
         <td>LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>B</td>
         <td>Drag-move refs</td>
         <td>B + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Alt</td>
         <td>Scale shadow depth bias</td>
         <td>Alt + B + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>B</td>
         <td>Drag-move refs</td>
         <td>Alt + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>S</td>
         <td>Scale FOVs</td>
         <td>Alt + S + LMB</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Ctrl</td>
         <td>Scale light fade</td>
         <td>Ctrl + Alt + S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>Alt</td>
         <td>Scale refs</td>
         <td>Ctrl + S + LMB</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>LMB</td>
         <td></td>
         <td>Ctrl + S</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>Ctrl</td>
         <td></td>
         <td>S</td>
         <td>Press [Ctrl + S] does not trigger.</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>S</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Alt</td>
         <td></td>
         <td>Alt</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>1</td>
         <td></td>
         <td>Alt + 1</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>1</td>
         <td>Press [Alt + 1]</td>
         <td>Alt</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>Alt</td>
         <td></td>
         <td></td>
         <td>Press Alt does not trigger.</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>Ctrl</td>
         <td></td>
         <td>Ctrl</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>A</td>
         <td></td>
         <td>Ctrl + A</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>A</td>
         <td>Select all</td>
         <td>Ctrl</td>
         <td>Press [Ctrl + A] triggers.</td>
      </tr>
      <tr>
         <td>keydown</td>
         <td>S</td>
         <td></td>
         <td>Ctrl + S</td>
      </tr>
      <tr>
         <td>keyup</td>
         <td>S</td>
         <td>Save</td>
         <td>Ctrl</td>
         <td>Press [Ctrl + S] triggers.</td>
      </tr>
   </tbody>
</table>

In the table above, we can observe three behaviors:

a. It's possible to seamlessly switch between multiple input sequences that have the Hold press type, just by pressing or releasing the keys unique to each input sequence. However, you cannot seamlessly switch from a Hold input sequence to a Press or Long Press input sequence: releasing the scale/light input controls will not inadvertently trigger the "Save" action, even if Ctrl + S are the last keys to be released and are released in that order.

b. Similarly, if one input sequence is a superset of another, then releasing the superset does not count as releasing the subset. The Press [Alt + 1] input sequence is a superset of the Press Alt input sequence, but if we release the 1 key first, releasing the Alt key does not trigger the Press Alt action.

c. However, we don't on principle forbid a single input control from being consumed by multiple input sequences. You can trigger the Press [Ctrl + A] input sequence and the Press [Ctrl + S] input sequence using the same Ctrl keypress, without having to release and re-press Ctrl between the two actions. This behavior is particularly important for modifier keys on gamepads.

Behavior A is the result of [Hold-blocks-Press](#simultaneous-release-of-hold-and-press-binds).

Behavior B is handled by the [specificity rule](#specificity-rule) in conflict resolution.

Behavior C is handled by application of the specificity rule during [conflict resolution across time](#conflict-resolution-across-time).

<span style="page-break-after: always"></span>

# Algorithms

## Merging of a parent and child input sequence

Given an [input sequence](#input-sequence) <var>Parent</var> and an input sequence <var>Child</var>, the process for merging <var>Child</var> into <var>Parent</var> is as follows:

1. Let <var>ChildRoot</var> be the root input sequence group of <var>Child</var>.
2. If <var>ChildRoot</var> is a null pointer, then return.
3. Let <var>FinalGroup</var> be the [final ISG](#final-isg) in <var>Parent</var>, and let <var>FinalGroupParent</var> be the parent of the final ISG.
4. If <var>FinalGroup</var> is a null pointer, then return.
5. Let <var>Wrap</var> be a null ISG pointer.
6. Assert: <var>FinalGroup</var>'s type is not *concurrent and ordered* or *separated and ordered*.
7. Assert: At most one of <var>Parent</var> and <var>Child</var> have a [range constraint](#range-constraint).
8. If <var>Child</var> has a range constraint, then overwrite <var>Parent</var>'s [lack of a] range constraint with it.
9. If <var>FinalGroupParent</var> is not null, and if it is an ordered group:
   1. If <var>FinalGroupParent</var> and <var>ChildRoot</var> have the same type:
      1. For each child group <var>Item</var> of <var>ChildRoot</var>:
         1. Clone <var>Item</var>, and append the clone to <var>FinalGroupParent</var> as a new child.
   1. Else:
      1. Clone <var>ChildRoot</var>, and append the clone to <var>FinalGroupParent</var> as a new child.
   1. Return.
10. Set <var>Wrap</var> to a new input sequence group, with the type *concurrent and ordered*.
11. Append <var>FinalGroup</var> to <var>Wrap</var> as a child.
12. Clone the root input sequence group in <var>Child</var>, and append the clone to <var>Wrap</var> as a child.
13. If <var>FinalGroupParent</var> is not null:
   1. Let variable <var>Index</var> be the index of <var>FinalGroup</var> in <var>FinalGroupParent</var>.
   1. Assert: <var>Index</var> is in bounds.
   1. Set the <var>Index</var>-th child of <var>FinalGroupParent</var> to <var>Wrap</var>.
14. Else:
   1. Assert: <var>FinalGroup</var> is the root input sequence group of <var>Parent</var>.
   1. Set the root input sequence group of <var>Parent</var> to <var>Wrap</var>.
15. Return.

## Identification of an input sequence's terminal inputs
The algorithm to gather up the [terminal inputs](#terminal-inputs) of an [input sequence](#input-sequence) are as follows:

1. Let <var>Group</var> be the input sequence's root group.
2. Let <var>Result</var> be a list to append to.
3. Process <var>Group</var>:
   1. If <var>Group</var>'s type is *single input control*, then append its input control to <var>Result</var>.
   1. Else if <var>Group</var>'s type is *separate and ordered*, then:
      1. If <var>Group</var> has any children, then run step 3 recursively on its last child.
   1. Else:
      1. For each child <var>Child</var> of <var>Group</var>:
         1. Run step 3 recursively on <var>Child</var>. 

## Generation of a bind list from a bind tree
Given a [bind tree](#bind-list) <var>SourceTree</var>, the steps for generating a [bind list](#bind-list) are as follows:

1. Let <var>Result</var> be an empty bind list.
1. Let <var>CurrentEditorMode</var> be null.
1. Let <var>CurrentNode</var> be <var>SourceTree</var>'s root node.
1. **[Traversal sub-algorithm.]** For each child <var>ChildNode</var> of <var>CurrentNode</var>:
   1. If <var>ChildNode</var> is a bound tool node:
      1. Let <var>NewBindListItem</var> be a new [bind list item](#bind-list-item).
      1. Set <var>NewBindListItem</var>'s input sequence to <var>ChildNode</var>'s absolute input sequence.
      1. Set <var>NewBindListItem</var>'s button press type to <var>ChildNode</var>'s button press type.
      1. Set <var>NewBindListItem</var>'s bound tool and associated options to <var>ChildNode</var>'s bound tool and associated options.
      1. Append <var>NewBindListItem</var> to <var>Result</var>.
      1. Skip to the next iteration.
   1. If <var>ChildNode</var> has no children, then skip to the next iteration.
   1. If <var>ChildNode</var> is an editor mode node:
      1. If <var>CurrentEditorMode</var> is null:
         1. Set <var>CurrentEditorMode</var> to <var>ChildNode</var>'s editor mode.
      1. Else:
         1. If <var>CurrentEditorMode</var> is not equal to <var>ChildNode</var>'s editor mode, then skip to the next iteration.[^2]
   1. Recursively execute this sub-algorithm given <var>ChildNode</var> as the new stack frame's <var>CurrentNode</var>.

[^2]: If an input node has multiple editor mode nodes as ancestors, and these editor mode nodes specify different editor modes, then the input node is impossible to trigger and so should be excluded from the bind list.

## Computation of an input node's absolute input sequence
An [input node](#input-node) <var>Node</var>'s absolute input sequence is defined as follows:

1. Let <var>Ancestor</var> be <var>Node</var>'s parent node.
2. While <var>Ancestor</var> is not null:
   1. Let <var>Casted</var> be <var>Ancestor</var> if <var>Ancestor</var> is an input node, or null otherwise.
   1. If <var>Casted</var> is not null:
      1. Let <var>Parent</var> be the absolute input sequence of <var>Casted</var>.
      1. Return, as a result, the [merging](#merging-of-a-parent-and-child-input-sequence) of <var>Parent</var> with <var>Node</var>'s input sequence.
1. Return a clone of <var>Node</var>'s input sequence.

## Per-frame input processing
1. Let <var>CurrentTimestamp</var> be the current time.
1. **[Update input device states.]** For every input device <var>Device</var>:
   1. Run device-specific code to update the known input state; for example, store all key states for a keyboard, or retrieve an up-to-date `XINPUT_GAMEPAD` for a gamepad.
   1. For every button <var>Button</var> in <var>Device</var>:
      1. Let <var>WasDown</var> be <var>Button</var>'s "is down" flag.
      1. Clear <var>Button</var>'s "is down" and "down state changed on this frame" flags.
      1. Clear the [down state changed on this frame](#device-button-state) flag.
      1. If the physical button is down:
         1. Set <var>Button</var>'s "is down" flag.
         1. If <var>WasDown</var> is *false*, then:
            1. Set the down timestamp to <var>CurrentTimestamp</var>.
            1. Set the "down state changed on this frame" flag.
      1. Else:
         1. If <var>WasDown</var> is *true*, then set the "down state changed on this frame" flag.
      1. If <var>Button</var> is not down, and <var>WasDown</var> is *false*, then:
         1. Reset <var>Button</var>'s [previous-frame claim](#device-button-state).
         1. Delete <var>Button</var>'s stored [raycast result](#raycast-result), if any.
      1. Else:
         1. If <var>Button</var>'s current-frame claim has a greater or equal [specificity](#specificity) than <var>Button</var>'s previous-frame claim:
            1. Overwrite <var>Button</var>'s previous-frame claim with <var>Button</var>'s current-frame claim.
      1. Reset <var>Button</var>'s current-frame claim.
1. Run the [bind list update algorithm](#bind-list-update-algorithm).
1. Deliver all queued (and merged, if applicable) bound function responses to the outside world i.e. to Worldedit.[^3]

[^3]: Algorithms in this document describe Worldinput as executing input nodes' bound tools. This is a simplification. In reality, Worldinput passes parameters to those tools as appropriate and receives queued actions, merging them if a tool is triggered more than once in a single frame. The queued actions are then delivered to the outside world at the end of input processing. In other words: we're not running editor operations in the middle of processing input; we do all input processing, and then we actually run the editor operations that the user's inputs triggered.

## Bind list update algorithm
The operations here are as follows:

* **Traversal.** We examine the full bind list and gather up all inputs that are eligible to activate.
* **Hold release.** If any of the "Hold" binds that were active in the previous frame are no longer active now, then we send a key-up notification to their respective bound tools.
* **Execution.** We examine all of the gathered eligible nodes, and execute their respective bound tools.

The algorithm works as follows.

* Track the Hold binds that were active in the last frame.
* Gather up every bind list item that is: a Hold bind which is down or has been released; or a non-Hold bind that has been released.
   * Apply conflict resolution rules as necessary while gathering binds.
* Process any Hold binds that were active in the last frame and have been released (or blocked by a conflict) on this frame.
* Execute all non-conflicting binds that are active.

Let <var>LastFrameActiveHoldBinds</var> be a persistent run-time-only list of [bind list items](#bind-list-item).

1. Let <var>EligibleBinds</var> be an empty list of bind list items.
2. Let <var>ConflictLosingBinds</var> be an empty list of bind list items.
3. Let <var>InterruptionCheck</var> be an [interruption check](#input-sequence-group-interruption-check) instance, initialized as follows:
   1. For every button <var>Button</var> on the current input device:
      1. If <var>Button</var> is currently down, then insert a new entry into <var>InterruptionCheck</var>'s buttons list, consisting of the identifying information for <var>Button</var> and the timestamp at which it went down.
4. For each bind list item <var>Current</var>:
   1. Let <var>Matched</var> be a bool, and set it to *false*.
   1. Run the [input sequence update algorithm](#input-sequence-update-algorithm) for <var>Current</var>'s [input sequence](#input-sequence), passing the current timestamp, the current input device, and <var>InterruptionCheck</var> as parameters.
   1. If <var>Current</var>'s input sequence now has a [frame status](#frame-status) of *down*:
      1. If <var>Current</var>'s [press type](#button-press-type) is Hold:
         1. Set <var>Matched</var> to *true*.
   1. Else if <var>Current</var>'s input sequence now has a frame status of *released*:
      1. If <var>Current</var>'s press type is Press:
         1. Set <var>Matched</var> to *true*.
      1. Else if <var>Current</var>'s press type is Long Press:
         1. Let <var>DownFor</var> be a timestamp equal to <var>CurrentTime</var> minus the [down timestamp](#down-timestamp) for <var>Current</var>'s input sequence.
         2. If <var>DownFor</var> is longer than the [long press time threshold](#button-press-type), then set <var>Matched</var> to *true*.
   1. If <var>Matched</var> is *false* and <var>Current</var>'s button press type is Hold:
      1. Modify <var>Current</var>'s state: clear the flags which indicate that it has been subject to the Press-blocks-Hold rule.
   1. If <var>Current</var> specifies an editor mode, and we are not in that editor mode, then set <var>Matched</var> to *false*.
   1. If <var>Matched</var> is *true* and <var>Current</var>'s input sequence has a [range constraint](#range-constraint):
      1. Let <var>ConstraintSatisfied</var> be the result of running the [input sequence range constraint check](#input-sequence-range-constraint-check) on <var>Current</var>'s input sequence.
      1. If <var>ConstraintSatisfied</var> is *false*, then set <var>Matched</var> to *false*.[^4]
   1. If <var>Matched</var> is *true*:
      1. Add <var>Current</var> to <var>EligibleBinds</var>.[^5]
      1. Let <var>CurrentConflicted</var> be a bool, and set it to *false*.
      1. For each node <var>PriorBind</var> in <var>EligibleBinds</var>:
         1. Run the [bind conflict resolution algorithm](#bind-conflict-resolution-algorithms) for [concurrent node conflicts](#concurrent-bind-conflict-resolution), given <var>Current</var> and <var>PriorBind</var> as the nodes to test. Let <var>WinningBind</var> be the returned node, and let <var>WinnerIsNotBlocked</var> be the returned Boolean.
         2. Let <var>LosingBind</var> be whichever of <var>Current</var> and <var>PriorBind</var> is not <var>WinningBind</var>.
         3. If <var>LosingBind</var>'s press type is not Hold[^6], then add <var>LosingBind</var> to <var>ConflictLosingBinds</var>.
         4. If <var>WinningBind</var> is Child, then:[^7]
            1. If <var>WinnerIsNotBlocked</var> is *false*, then set <var>CurrentConflicted</var> to *true*.
         5. If <var>WinningBind</var> is <var>PriorBind</var>, then set <var>CurrentConflicted</var> to *true*.
      1. If <var>CurrentConflicted</var> is *true*:
         1. If <var>Current</var>'s press type is not Hold, then add <var>Current</var> to <var>ConflictLosingBinds</var>.
         2. Skip to the next iteration (the next bind list item).
5. For every node <var>Conflicted</var> in <var>ConflictLosingBinds</var>:
   1. Remove <var>Conflicted</var> from <var>EligibleBinds</var>, if it is present in that list.
   1. If <var>Conflicted</var>'s press type is not Hold[^8], then [clear all progress](#clear-all-progress) for <var>Conflicted</var>'s input sequence.
6. Clear all elements out of <var>ConflictLosingBinds</var>.
7. **[Press-preempts-Hold.]** Enforce the Press-preempts-Hold rules.
   1. Let <var>SeenHoldBinds</var> be a list of data structures. Each data structure refers to a Hold bind list item, and tracks: how many Press bind list items it outlasted; how many Press bind list items it was delayed by; whether it was blocked by any Press bind list item; and whether it was advanced past by any Press bind list item.
   1. Let <var>WinningPressBinds</var> be an empty list of bind list items.
   1. For each entry <var>EligibleBind</var> in <var>EligibleBinds</var>:
      1. If <var>EligibleBind</var>'s button press type is Hold:
         1. Let <var>Entry</var> be a new entry inserted into <var>SeenHoldBinds</var>.
         2. Set <var>Entry</var>'s node pointer to <var>EligibleBind</var>.
         3. If <var>EligibleBind</var>'s state indicates that it's subject to Press-blocks-Hold, then set <var>Entry</var>'s "is blocked" flag to *true*.
   1. If <var>SeenHoldBinds</var> is not empty:
      1. For each bind list item <var>Current</var>:
         1. If <var>Current</var>'s button press type is not Hold:
            1. For every entry <var>Entry</var> in <var>SeenHoldBinds</var>:
               1. Let <var>Result</var> be the result of running the [bind conflict resolution algorithm](#bind-conflict-resolution-algorithms) for [Presses preempting Holds](#press-preempts-hold), given <var>Current</var> as the "press" bind and <var>Entry</var>'s bind pointer as the "hold" bind.
               1. If <var>Result</var> is *delayed* or *advanced past*, then
                  1. Increment <var>Entry</var>'s "delayed by" count by 1, and add <var>Current</var> to <var>WinningPressBinds</var>.
               1. If <var>Result</var> is *advanced past*, then set <var>Entry</var>'s "is advanced past" flag to *true*.
               1. If <var>Result</var> is *blocked*, then set <var>Entry</var>'s "is blocked" flag to *true*.
               1. If <var>Result</var> is *hold outlasted press* or *hold won via range*, then increment <var>Entry</var>'s "outlasted" count by 1.
      1. Let <var>RetroamendedLosingPresses</var> be an empty list of bind list items.
      1. **[Retroamend losing presses.]** For each entry <var>Entry</var> in <var>SeenHoldBinds</var>:
         1. If <var>Entry</var>'s "is blocked" or "is advanced past" flags are set, then skip to the next iteration.
         2. If <var>Entry</var>'s "outlasted" count is zero, then skip to the next iteration.
         3. Increase <var>Entry</var>'s "outlasted" count by its "delayed by" count.
         4. Set <var>Entry</var>'s "delayed by" count to zero.
         5. For each node <var>PressBind</var> in <var>WinningPressBinds</var>:
            1. Let <var>Result</var> be the result of running the [bind conflict resolution algorithm](#bind-conflict-resolution-algorithm) for [Presses preempting Holds](#press-preempts-hold), given <var>PressBind</var> as the "press" bind and <var>Entry</var>'s bind pointer as the "hold" bind.
            1. If <var>Result</var> is *delayed* or *hold outlasted press* or *hold won via range*, then add <var>PressBind</var> to <var>RetroamendedLosingPresses</var>.
      1. Remove from <var>WinningPressBinds</var> every bind that is also in <var>RetroamendedLosingPresses</var>.
      1. **[Salvage Hold binds that lost to a losing Press bind.]** For each <var>Entry</var> in <var>SeenHoldBinds</var>:
         1. If <var>Entry</var>'s "is blocked" flag is set, then skip to the next iteration.
         2. If <var>Entry</var>'s "delayed by" count is zero, then skip to the next iteration.
         3. Let <var>AnyLoss</var> be *false*.
         4. Let <var>StillAdvancedPast</var> be *false*.
         5. For each bind <var>PressBind</var> in <var>WinningPressBinds</var>:
            1. Let <var>Result</var> be the result of running the [bind conflict resolution algorithm](#bind-conflict-resolution-algorithm) for [Presses preempting Holds](#press-preempts-hold), given <var>PressBind</var> as the "press" bind and <var>Entry</var>'s bind pointer as the "hold" bind.
            1. If <var>Result</var> is delayed, then set <var>AnyLoss</var> to *true*.
            1. Else if <var>Result</var> is advanced past, then set <var>AnyLoss</var> and <var>StillAdvancedPast</var> to *true*.
         6. If <var>AnyLoss</var> is *false*:
            1. Set <var>Entry</var>'s "delayed by" count to zero.
            1. If <var>StillAdvancedPast</var> is *false*, then clear <var>Entry</var>'s "is advanced past" flag.
      1. **[Disqualify all still-losing Hold nodes.]** For each entry <var>Entry</var> in <var>SeenHoldBinds</var>:
         1. If <var>Entry</var>'s "is blocked" and "is advanced past" flags are clear, and its "delayed by" count is zero:
            1. If <var>Entry</var>'s node's state does not indicate that it's subject to Press-blocks-Hold:
               1. Skip to the next iteration.
         2. Remove <var>Entry</var>'s bind list item from <var>EligibleBinds</var>.
1. **[Range control conflicts.]** Begin checking for range control conflicts.
   1. For each entry <var>Bind</var> in <var>EligibleBinds</var>:
      1. Update the state for <var>Bind</var>: clear the range axis "blocked" flags.
   1. Iterate over <var>EligibleBinds</var> with a nested loop, comparing every bind to every other bind. Let <var>A</var> and <var>B</var> be any two binds being compared; for each such pair:
      1. Run the [range conflict resolution algorithm](#range-conflict-resolution) on <var>A</var> and <var>B</var>.
   1. For each entry <var>Bind</var> in <var>EligibleBinds</var>:
      1. If <var>Bind</var>'s input sequence has a [range constraint](#range-constraint):
         1. Check <var>Bind</var>'s range conflict state: if all of the axes specified in <var>Bind</var>'s range constraint are flagged as blocked, then remove <var>Bind</var> from <var>EligibleBinds</var>.
8. **[Hold release.]** Process any Hold binds that were previously held and have now been released:
   1. For each entry <var>HoldBind</var> in <var>LastFrameActiveHoldBinds</var>:
      1. If <var>HoldBind</var> is not in <var>EligibleBinds</var>:
         1. Execute its bound tool in the context of a key-up.
9. **[Holds blocking Presses.]** For each entry <var>PressBind</var> in <var>EligibleBinds</var>:
   1. If <var>PressBind</var>'s button press type is neither Press nor Long Press, then skip to the next iteration.
   1. For each entry <var>ReleasedHoldBind</var> in <var>LastFrameActiveHoldBinds</var>:
      1. If <var>ReleasedHoldBind</var> is in <var>EligibleBinds</var>, then skip to the next iteration.
      1. Let <var>Result</var> be the result of running the [bind conflict resolution algorithm](#bind-conflict-resolution-algorithm) for [holds blocking presses](#holds-blocking-presses), given <var>PressBind</var> as the "press" bind and <var>ReleasedHoldBind</var> as the "hold" bind.
      1. If <var>Result</var> is *true*:
         1. Remove <var>PressBind</var> from <var>EligibleBinds</var>.
         2. Break.
10. Set <var>LastFrameActiveHoldBinds</var> to an empty list.
11. **[Execution.]** For each entry <var>EligibleBind</var> in <var>EligibleBinds</var>:
   1. Let <var>RangeValue</var> be a 2D vector with floating-point components initialized to zero.
   1. Let <var>RangeIsDelta</var> be *false*.
   1. Let <var>RangeIsStale</var> be *false*.
   1. If <var>EligibleBind</var> has a [range constraint](#range-constraint):
      1. If the range control is a [delta control](#delta-controls), then set <var>RangeIsDelta</var> to *true*.
      1. Set <var>RangeValue</var> to the value of the range control along the axes specified by the range constraint.
      1. If the range control is a delta control:
         1. If the range control is stale, then set <var>RangeIsStale</var> to *true*.
      1. If <var>EligibleBind</var> has any range control axes flagged as "blocked" (i.e. due to losing range control conflicts):
         1. For each blocked axis, modify <var>RangeValue</var> to set the corresponding axis magnitude to zero.
         1. If <var>RangeValue</var> is zero on all axes:
            1. Skip all further processing for <var>EligibleBind</var>, and proceed to the next loop iteration.
   1. If <var>RangeIsStale</var> is *false*:
      1. Let <var>Cause</var> be a tool request cause.
      1. If <var>EligibleBind</var>'s input sequence contains any buttons, then set <var>Cause</var>'s "has button" flag to *true*.
      1. If <var>EligibleBind</var> has a range constraint, then set <var>Cause</var>'s "has range" flag to *true*.
      1. If <var>EligibleBind</var>'s button press type is Hold, then set <var>Cause</var>'s "button is down" flag to *true*.
      1. Set <var>Cause</var>'s "button went down at" timestamp to the [down timestamp](#down-timestamp) of <var>EligibleBind</var>'s input sequence.
      1. Set <var>Cause</var>'s button press type to <var>EligibleBind</var>'s button press type.
      1. Set <var>Cause</var>'s "down status changed this frame" flag to the frame status change flag of <var>EligibleBind</var>'s input sequence.
      1. If <var>EligibleBind</var>'s press type is Hold:
         1. If the frame status change flag of <var>EligibleBind</var>'s input sequence is not set:
            1. If <var>EligibleBind</var> is in <var>LastFrameActiveHoldBinds</var>, then set <var>Cause</var>'s "down status changed this frame" flag.
      1. Set <var>Cause</var>'s range value to <var>RangeValue</var>.
      1. Set <var>Cause</var>'s "range is delta" flag to <var>RangeIsDelta</var>.
      1. If <var>EligibleBind</var>'s input sequence has a [raycast constraint](#raycast-constraint):
         1. Let <var>RaycastResult</var> be either of...
            1. The [raycast result](#raycast-result) for the current frame, if <var>EligibleBind</var>'s range constraint requires that the raycast be updated per-frame.
            1. Otherwise, the [raycast result](#raycast-result) associated with the constraint's raycast-associated button.
         2. Set <var>Cause</var>'s raycast result to a copy of <var>RaycastResult</var>.
      1. Execute <var>EligibleBind</var>'s bound tool, passing <var>Cause</var>.
   1. If <var>EligibleBind</var>'s press type is Hold, then:
      1. Add <var>EligibleBind</var> to <var>LastFrameActiveHoldBinds</var>.
   1. Else:
      1. Assert: <var>EligibleBind</var>'s input sequence has a frame status of *released*.
      1. Assert: All groups in <var>EligibleBind</var>'s input sequence have a frame status of *inactive*, as they should've been cleared at the end of the input sequence update algorithm.
      1. Set <var>EligibleBind</var>'s frame status to *inactive*.

[^4]: The reason this isn't integrated into the input sequence update algorithm is because we want a Hold bind to clear its Press-blocked-Hold flag when the buttons are released, but not when the range constraint ceases to be met. Range constraints are not intended to influence the handling of button press type conflicts, nor can they do so reliably: a very slow mouse movement, for example, may be a contiguous physical movement, but because mouse coordinates are measured in integer pixels, it will register as a series of starts and stops depending on the timing at which the cursor crosses pixel boundaries; and so such a movement would improperly clear the Press-blocked-Hold flag if range constraint failures allowed such flags to be cleared.
[^5]: We add <var>Child</var> to <var>EligibleBinds</var> even if it loses a conflict just as a lazy shortcut to run conflict tests against conflict losers (i.e. if <var>A</var> loses a conflict to <var>B</var>, it should be possible for <var>C</var> to lose a conflict to <var>A</var> even if <var>C</var> does not directly lose to <var>B</var>). Essentially, <var>EligibleBinds</var> represents binds that are eligible for potentially activating if they don't get disqualified by losing a conflict.
[^6]: In order to allow seamless switching between Hold binds, we do not clear a Hold bind's key sequence progress if the bind is blocked by the activation of a [more specific](#specificity-rule) Hold bind.
[^7]: Note that we do not here remove conflict losers from <var>EligibleBinds</var>; we just add them to <var>ConflictLosingBinds</var>. This is because we still want to test nodes that we encounter in the future against them for conflicts. After traversal, we'll remove all <var>ConflictLosingBinds</var> nodes from <var>EligibleBinds</var>.
[^8]: If we clear all progress on Hold binds, then it becomes impossible to seamlessly switch between input sequences like Hold [X + Y] and Hold [X + (Y + Z)]. If the latter input sequence (being [more specific](#specificity-rule)) interrupts the former, then clearing all progress on the former would prevent it from being considered *down* again unless the X key is released and re-pressed.

## Bind conflict resolution algorithms
There are multiple kinds of conflicts that can occur between binds, and so there are multiple algorithms for checking whether two binds are in conflict and if so, which bind should win that conflict.

### Press-preempts-Hold
Given two bind list items &mdash; one, <var>PressBind</var>, whose [button press type](#button-press-type) is Press or Long Press; and another, <var>HoldBind</var>, whose button press type is Hold and whose current [frame status](#frame-status) is *down* &mdash; this algorithm determines whether and how the press should affect the hold, [as per our desired conflict resolution rules](#press-preempts-hold). The algorithm returns an enumeration with the following possible values:

* **no conflict:** No conflict is present between these binds.
* **delayed:** A conflict is present between these binds, and the Press bind has won; activation of the Hold bind will be delayed for a finite amount of time.
* **blocked:** A conflict is present between these binds, the Press bind has won, and the Press bind is being released; activation of the Hold bind will be blocked outright.
* **advanced past:** A conflict is present between these binds, and the Press bind has won by virtue of advancing past the Hold bind; activation of the Hold bind will be delayed indefinitely.
* **hold outlasted press:** A conflict is present between these binds, and the Hold bind has won.
* **hold won via range:** A conflict is present between these binds, and the Hold bind has won due to having a range constraint become active.

This algorithm requires access to: the timestamp at which input processing began for the current frame; and the current input device.

1. Let <var>CurrentTimestamp</var> be the timestamp at which input processing began for the current frame.
2. Let <var>SeqPress</var> be the input sequence of <var>PressBind</var>.
3. Let <var>SeqHold</var> be the input sequence of <var>HoldBind</var>.
4. Let <var>KeysHold</var> be the terminal inputs of <var>SeqHold</var>'s [final ISG](#final-isg).
5. If <var>PressBind</var>'s input sequence has a [frame status](#frame-status) of *released*, then:
   1. Let <var>KeysPress</var> be the terminal inputs of <var>PressBind</var>.
   1. If all input controls in <var>KeysHold</var> are also in <var>KeysPress</var>, then:
      1. Modify <var>HoldBind</var>'s state to indicate that it has become subject to Press-blocks-Hold.
      1. Return *blocked*.
6. Let <var>KeysPress</var> be an empty list of pairs, with each pair consisting of an input control and a "passed" bool (initialized to *false*).
7. Let <var>Current</var> be the root input sequence of <var>PressBind</var>.
8. **[Key-gathering sub-algorithm.]** Given <var>Current</var>, this sub-algorithm returns a frame status and a timestamp. Its purpose is to identify where in <var>PressBind</var>'s input sequence the user is &mdash; how much of the sequence they've already inputted &mdash; and to gather a list of all input controls up to and at that point.[^9] (This sub-algorithm will by necessity parallel the [input sequence group update algorithm](#input-sequence-group-update-algorithm); the overall logic will be the same.) If any of these controls are among the terminal inputs of <var>HoldBind</var>, then a conflict is present. Additionally, the sub-algorithm marks input controls that we have advanced past.[^10]
   1. If <var>Current</var>'s type is *single input control*:
      1. If the input control to which <var>Current</var> refers is currently down, then:
         1. Add that input control to <var>KeysPress</var>, paired with a "passed" bool of *false*.
         2. Return a frame status of *down*, and the timestamp at which the input control went down.
      1. Else, return a frame status of *inactive*, along with a zero timestamp.
   1. Else if <var>Current</var>'s type is *concurrent and ordered*:
      1. Let <var>PreviousTimestamp</var> be a zero timestamp.
      1. Let <var>PreviousStart</var> be the size of <var>KeysPress</var>, i.e. an index past that list's end.
      1. For each child <var>Child</var> of <var>Current</var>:
         1. Let <var>PriorCount</var> be the number of entries currently in <var>KeysPress</var>.
         2. Recursively run this sub-algorithm on <var>Child</var>. Let <var>ResultStatus</var> be the returned frame status, and let <var>ResultDownTimestamp</var> be the returned timestamp.
         3. If <var>ResultStatus</var> is down:
            1. If <var>ResultDownTimestamp</var> is prior to <var>PreviousTimestamp</var>, then:
               1. Truncate <var>KeysPress</var> to the first <var>PriorCount</var>-many entries.[^11]
               1. Return a frame status of *inactive*, along with <var>PreviousTimestamp</var>.
            1. For all indices from <var>PreviousStart</var> up to and not including <var>PriorCount</var>:
               1. Take the element of <var>KeysPress</var> at the current index, and set that element's "passed" bool to *true*.
         4. Else:
            1. Return a frame status of *inactive*, along with <var>PreviousTimestamp</var>.
      1. Return a frame status of *down*, along with <var>PreviousTimestamp</var>.
   1. Else if <var>Current</var>'s type is *concurrent and unordered*:[^12] 
      1. Let <var>AdditionsStartAt</var> be the <var>KeysPress</var>'s current size, i.e. an index past its end.
      1. Let <var>AnyNotDown</var> be false.
      1. Let <var>MostRecentTimestamp</var> be a zero timestamp.
      1. For each child <var>Child</var> of <var>Current</var>:
         1. Set <var>PriorCount</var> to <var>KeyPress</var>'s current size.
         2. Recursively run this sub-algorithm on <var>Child</var>. Let <var>ResultStatus</var> be the returned frame status, and let <var>ResultDownTimestamp</var> be the returned timestamp.
         3. If <var>ResultStatus</var> is not down, then set <var>AnyNotDown</var> to *true*.
         4. If <var>ResultDownTimestamp</var> is more recent than <var>MostRecentTimestamp</var>, then:
            1. Set <var>MostRecentTimestamp</var> to <var>ResultDownTimestamp</var>.
            1. For each index from <var>AdditionsStartAt</var> up to, and not including, <var>PriorCount</var>:
               1. Take the element of <var>KeysPress</var> at the current index, and set that element's "passed" bool to *true*.
            1. Set <var>AdditionsStartAt</var> to <var>PriorCount</var>. [^13]
         5. Else:
            1. For each element of <var>KeysPress</var> starting at index <var>PriorCount</var>:
               1. Set the current element's "passed" bool to *true*.
      1. If <var>AnyNotDown</var> is *true*, then return a frame status of *inactive*, along with <var>MostRecentTimestamp</var>.
      1. Return a frame status of *down*, along with <var>MostRecentTimestamp</var>.
   1. Else if <var>Current</var>'s type is *separate and ordered*:[^14]
      1. If <var>Current</var> has no children, then return a frame status of *inactive*, along with a zero timestamp.
      1. Let <var>Child</var> be the child of <var>Current</var> at the index indicated by <var>Current</var>'s <var>CurrentItemIndex</var>.
      1. Recursively run this sub-algorithm on <var>Child</var>. Let <var>ResultStatus</var> be the returned frame status, and let <var>ResultDownTimestamp</var> be the returned timestamp.
      1. If <var>ResultStatus</var> is down and <var>Child</var> is the last child of <var>Current</var>, then return a frame status of *down*, along with <var>CurrentTimestamp</var>.
      1. Return a frame status of *inactive*, along with <var>ResultDownTimestamp</var>.
   1. Assert: unreachable.
9. Let <var>AllPassed</var> be true.
10. Let <var>AnyConflicted</var> be false.
11. For each key <var>KeyPress</var> in <var>KeysPress</var>:
    1. For each key <var>KeyHold</var> in <var>KeysHold</var>:
       1. If <var>KeyPress</var>'s button is the same button referred to by <var>KeyHold</var>:
          1. Set <var>AnyConflicted</var> to *true*.
          2. If <var>KeyPress</var>'s "passed" bool is *false*:
             1. Set <var>AllPassed</var> to *false*.
          3. Break.
12. If <var>AnyConflicted</var> is *false*:
    1. Return *no conflict*.
13. If <var>AllPassed</var> is *true*:[^15]
    1. Return *advanced past*.
1.  **[Hold nodes with range constraints win if the ranges are active.]** If <var>HoldBind</var>'s input sequence has a [range constraint](#range-constraint):
    1. If <var>PressBind</var>'s input sequence has a range constraint:
       1. Let <var>HoldRange</var> be the range constraint for <var>HoldBind</var>'s input sequence, and let <var>PressRange</var> be the range constraint for <var>PressBind</var>'s input sequence.
       1. If <var>HoldRange</var> and <var>PressRange</var> refer to different range controls, then return *no conflict*.
       1. If <var>HoldRange</var> and <var>PressRange</var> refer to different axes, then return *no conflict*.
    1. Else:
       1. Return *hold won via range*. (We do not have to check whether the range constraint is satisfied; it must have been, for <var>HoldBind</var> to end up here.)
14. Let <var>ElapsedHold</var> be the difference between the current timestamp and the down timestamp for <var>HoldBind</var>'s input sequence.
15. Let <var>DisambiguationDuration</var> be the press-to-hold time threshold.
16. If <var>PressBind</var>'s button press type is Long Press, then increase <var>DisambiguationDuration</var> by the long press time threshold.
17. If <var>ElapsedHold</var> is greater than <var>DisambiguationDuration</var>, then return *hold outlasted press*.
18. Return *delayed*.

[^9]: We cannot simply use <var>PressBind</var>'s terminal inputs, as this would fail to capture cases like the [En Route example](#en-route-example).
[^10]: As an example, consider the input nodes Press [A + B + X + Y] and Hold B. If you press and hold A, B, and X in that order, then you have advanced past the A and B buttons. If you release X, then you have advanced past only the A button.
[^11]: This has the effect of ensuring that <var>Child</var> and its descendants only contribute to <var>KeysPress</var> if <var>Child</var>'s various input controls went down in the proper order relative to its previous-sibling ISGs.
[^12]: For *concurrent and ordered* groups, an ISG and its keys are "passed" if the next ISG is down or partially down. By contrast, for *concurrent and unordered* groups, an ISG and its keys are "passed" if any sibling ISG is down or partially down and has a more recent down timestamp (i.e. "passed" status is determined based on the order in which the user presses down the child ISGs).
[^13]: Users can advance through the children of a *concurrent and unordered* group in any order. If they advance through those children in the same order we iterate over them, however, then this branch of the sub-algorithm will be hit for multiple children of the same group. If we did not update <var>AdditionsStartAt</var>, then we would in that case redundantly iterate over all <var>KeysPress</var> additions of all previous siblings.
[^14]: It is worth noting that we don't need to handle the "passed" bools for any entries in <var>KeysPress</var> that we add while processing a *concurrent and ordered* group. This is because you can only advance through a *concurrent and ordered* group by releasing keys. If one of the conflicting keys (between the Press and Hold nodes) is a child of a *concurrent and ordered* group, then you can't pass that key and still have that specific key be conflicting, because you have to release it to pass it, at which point (because conflicting keys are by definition among the terminal inputs of the Hold node) the Hold node ceases to be *down*.
[^15]: If all of the conflicting input controls were passed, then we delay the Hold node indefinitely. This ensures that you have unlimited time to enter the rest of the (Long) Press node's input sequence.

### Concurrent bind conflict resolution
Given two binds <var>A</var> and <var>B</var> that are activating on the same frame (i.e. binds whose button press types are Press or Long Press, and whose input sequences' frame statuses are both released; or binds whose button press types are Hold, and whose input sequences' frame statuses are both down), this algorithm determines whether the binds are in conflict, and if so, which bind, if any, should win the conflict [given our desired conflict resolution rules](#simultaneous-activation-of-like-binds). The algorithm produces two result values: <var>WinningBind</var>, a bind list item pointer; and <var>AllowActivation</var>, a Boolean.

1. Set <var>WinningBind</var> to a null pointer.
2. Set <var>AllowActivation</var> to *true*.
3. If either, but not both, of <var>A</var> and <var>B</var> have a button press type of Hold, then return.
4. Let <var>SequenceA</var> be the input sequence for <var>A</var>.
5. Let <var>SequenceB</var> be the input sequence for <var>B</var>.
6. If <var>SequenceA</var> and <var>SequenceB</var> are equivalent:
   1. If <var>A</var> and <var>B</var> have the same button press type:
      1. Return.
   1. If neither <var>A</var> nor <var>B</var> use Hold as their button press type:
      1. Let <var>PressBind</var> be whichever of <var>A</var> or <var>B</var> uses Press as its button press type, and <var>LongPressBind</var> be the other bind.
      1. Let <var>Elapsed</var> be the difference between the [down timestamp](#down-timestamp) for <var>LongPressBind</var>'s input sequence, and the current timestamp.
      1. If <var>Elapsed</var> is greater than the [long press time threshold](#button-press-types), then:
         1. Set <var>WinningBind</var> to <var>LongPressBind</var>.
      1. Else:
         1. Set <var>WinningBind</var> to <var>PressBind</var>.
      1. Return.
7. If either, but not both, of <var>A</var> and <var>B</var> use the Hold press type, then return.
8. **[Specificity rule.]** If the input sequences for <var>A</var> and <var>B</var> do not have equal [specificity](#specificity):
   1. Let <var>MoreSpecificBind</var> be whichever of <var>A</var> and <var>B</var> has an input sequence with greater specificity. Let <var>LessSpecificBind</var> be the other bind.
   1. If <var>MoreSpecificBind</var> and <var>LessSpecificBind</var> have the same [range constraint](#range-constraint):[^16]
      1. Set <var>WinningBind</var> to <var>MoreSpecificBind</var>.
      1. Return.
   1. Let <var>SubsetFinalISG</var> be the [final ISG](#final-isg) of <var>LessSpecificBind</var>'s input sequence.
   1. If <var>SubsetFinalISG</var> is a single input control:
      1. If that input control is one of <var>MoreSpecificBind</var>'s terminal inputs:
         1. Set <var>WinningBind</var> to <var>MoreSpecificBind</var>.
         2. Return.
   1. Else:
      1. If any of the input controls anywhere in <var>SubsetFinalISG</var> are one of <var>MoreSpecificBind</var>'s terminal inputs:
         1. Set <var>WinningBind</var> to <var>MoreSpecificBind</var>.
         2. Return.
9. **[Modifier conflict rule.]** If neither <var>A</var> nor <var>B</var> use Hold as their button press type, and their input sequences have equal specificity:
   1. If any input controls appear in both the terminal inputs for <var>A</var>, and the terminal inputs for <var>B</var>:
      1. Set <var>AllowActivation</var> to *false*.
      1. Return.
10. Return.

[^16]: In this situation only, we are handling the range constraint as if it were a terminal input; the logic, and the influence on our intended outcomes, are the same.

### Holds blocking Presses
Given two bind list items &mdash; one, <var>PressBind</var>, whose button press type is Press or Long Press; and another, <var>HoldBind</var>, whose button press type is Hold &mdash; whose input sequences' frame statuses are currently released, this algorithm determines whether the hold should block the press, [as per our desired conflict resolution rules](#simultaneous-release-of-hold-and-press-binds).

1. Let <var>SeqPress</var> be the terminal inputs of <var>PressBind</var>.
2. Let <var>SeqHold</var> be the terminal inputs of <var>HoldBind</var>.
3. If any input control is present in both <var>SeqPress</var> and <var>SeqHold</var>, then return true.
4. Return false.

### Range conflict resolution
Given two bind list items <var>A</var> and <var>B</var> that are potentially activating on the same frame (i.e. their button press type is Hold, and their input sequences are down), this algorithm determines whether the binds are in conflict, and if so, the nature of that conflict.

1. If either or both of <var>A</var> and <var>B</var> lack a [range constraint](#range-constraint), then return; the binds are not in conflict.
2. If the range requirements of <var>A</var> and <var>B</var> do not use the same [range control](#input-control), then return; the binds are not in conflict.
3. If neither of the range requirements of <var>A</var> nor <var>B</var> use all axes of their given range control, and if they don't use the same axis, then return; the binds are not in conflict.
4. If only one of the range requirements of <var>A</var> and <var>B</var> uses all axes of its given range control, then:
   1. Let <var>AllAxesBind</var> be whichever of <var>A</var> and <var>B</var> has a range requirement that uses all axes of its given range control. Let <var>SingleAxisBind</var> be the other bind.
   2. Let <var>ConflictAxis</var> be the axis used by <var>SingleAxisBind</var>.
   3. Update the state for <var>AllAxesBind</var>: mark <var>ConflictAxis</var> as blocked.
   4. Return.
5. Compare the [specificity](#specificity) of the input sequences for <var>A</var> and <var>B</var>.
6. If both binds have equal specificity, then return; the binds are not in conflict.
7. Update the state for whichever bind has a lower specificity: mark all axes as blocked.

## Input sequence group interruption check
This algorithm is invoked when updating a *separate and ordered* input sequence group. The algorithm requires the use of an instance of the following data structure, which is partially prepared by the [bind list update algorithm](#bind-list-update-algorithm) and then further prepared by the [input sequence update algorithm](#input-sequence-update-algorithm):

```c++
struct interruption_check {
   struct potentially_interrupting_button {
      const inputs::button button;
      const timestamp_t    down_at;
      bool matched = false;
   };

   std::vector<potentially_interrupting_button> buttons;
   size_t start_at = 0; 
};
```

The bind list update algorithm should initialize this data structure to list all buttons on the current input device that are currently down, and the timestamps at which they went down.

The input sequence update algorithm will further initialize this data structure as described in the respective section of this document.

The input sequence group interruption check runs only on *separate and ordered* groups, and returns a Boolean as a result: true if the group is interrupted, or false otherwise. The steps are as follows:

1. Let <var>Group</var> be the input sequence group we are running this algorithm on.
2. Let <var>InterruptionCheck</var> be the interruption check object passed to this algorithm.
3. Assert: <var>Group</var>'s type is *separate and ordered*.
4. If <var>Group</var>'s <var>CurrentItemIndex</var> is 0, then return false.
5. If <var>Group</var>'s <var>CurrentItemIndex</var> is equal to the index of its last child, then:
   1. **[Separate-and-ordered completion check sub-algorithm.]** Given <var>Group</var>:
      1. If <var>Group</var>'s type is single input control:
         1. For each button <var>Button</var> in <var>InterruptionCheck</var> &mdash; including buttons whose indices come before <var>InterruptionCheck</var>'s "start at" index &mdash; do:
            1. If <var>Button</var> is the input control that <var>Group</var> refers to:
               1. Return true.
         2. Return false.
      1. If <var>Group</var>'s type is *separate and ordered*:
         1. If <var>Group</var>'s <var>CurrentItemIndex</var> is less than the number of children in <var>Group</var> minus one (i.e. it's not the index of the last child), then return false.
         2. Return the result of recursively running this sub-algorithm on the last child of <var>Group</var>.
      1. For each child <var>Child</var> of <var>Group</var>:
         1. Let <var>Result</var> be the result of recursively running this sub-algorithm on <var>Child</var>.
         2. If <var>Result</var> is *false*, then return false.
      1. Return true.
   1. If [the top-level iteration of] the above sub-algorithm returns true for <var>Group</var>, then return false.
6. For each button <var>Button</var> in <var>InterruptionCheck</var>, starting from the index indicated by <var>InterruptionCheck</var>'s "start at" property:
   1. Set <var>Button</var>'s "matched" property to *false*.
7. Let <var>Current</var> be the child of <var>Group</var> at the index indicated by <var>Group</var>'s <var>CurrentItemIndex</var> value.
8. **[Traversal sub-algorithm.]** Given <var>Current</var>:
   1. If <var>Current</var>'s type is single input control:
      1. For each button <var>Button</var> in <var>InterruptionCheck</var>, starting from the index indicated by <var>InterruptionCheck</var>'s "start at" property:
         1. If <var>Button</var> is the input control that <var>Current</var> refers to:
            1. Set <var>Button</var>'s "matched" property to *true*.
            1. Break.
      1. Return.
   1. If <var>Current</var>'s type is *separate and ordered*:
      1. Let NextItem be the child of <var>Current</var> at the index indicated by <var>Current</var>'s <var>CurrentItemIndex</var> value.
      1. Recursively execute this sub-algorithm with NextItem as the next iteration's value of <var>Current</var>.
      1. Return.
   1. For each item <var>Item</var> in <var>Current</var>:
      1. Recursively execute this sub-algorithm with <var>Item</var> as the next iteration's value of <var>Current</var>.
9. For each button <var>Button</var> in <var>InterruptionCheck</var>, starting from the index indicated by <var>InterruptionCheck</var>'s "start at" property:
   1. If <var>Button</var>'s "matched" property is *false*, then return true.
10. Return false.

## Input sequence update algorithm
This algorithm is potentially invoked by the bind list update algorithm, running at most once per frame for any given input sequence. This algorithm takes as input the timestamp at which input processing began for the current frame, the current input device handler, and an interruption check instance.

1. Let <var>CurrentTimestamp</var> be the timestamp at which input processing began for this frame.
2. Let <var>InterruptionCheck</var> be the [interruption check](#input-sequence-group-interruption-check) instance received as a parameter.
3. If this input sequence is empty, then:
   1. If this input sequence has a [range constraint](#range-constraint):[^17]
      1. Set this input sequence's [frame status](#frame-status) to *down*.
      1. Clear this input sequence's frame status change flag.
   1. Abort this process.
1. If this input sequence's [raycast success flag](#raycast-success-flag) is set:
   1. Let <var>Button</var> be this input sequence's [raycast-associated button](#raycast-constraint).
   1. If <var>Button</var> is not [down](#device-button-state):
      1. Clear this input sequence's raycast success flag.
4. Set up the interruption check:
   1. Set <var>InterruptionCheck</var>'s "start at" property to 0.
   1. Let <var>FoundFirstButton</var> be false.
   1. For each button <var>Button</var> on <var>InterruptionCheck</var>:
      1. If <var>Button</var>'s "down at" property is more recent than this input sequence's [last advancement time](#last-advancement-time):
         1. Set <var>FoundFirstButton</var> to *true*.
         2. Set <var>InterruptionCheck</var>'s "down at" property to the index of <var>Button</var> in <var>InterruptionCheck</var>'s button list.
         3. Break.
   1. If <var>FoundFirstButton</var> is *false*:
      1. Set <var>InterruptionCheck</var>'s "down at" property to the size of its button list.[^18]
5. Let <var>RootGroup</var> be the root group of this input sequence.
6. Run the [input sequence group update algorithm](#input-sequence-group-update-algorithm) on <var>RootGroup</var>. Let <var>ResultStatus</var> be the returned frame status; let <var>ResultDownTimestamp</var> be the returned timestamp; let <var>ResultCount</var> be the returned count; and let <var>ResultRaycastStatus</var> be the returned raycast status.
1. If this input sequence has a [raycast constraint](#raycast-constraint):
   1. If <var>ResultRaycastStatus</var> is *raycast requirement met*:
      1. Set this input sequence's raycast success flag.
   1. Else if <var>ResultRaycastStatus</var> is *raycast requirement failed*:
      1. Clear this input sequence's raycast success flag.
      1. If <var>ResultStatus</var> is *released*:
         1. [Clear all progress](#clear-all-progress) for this input sequence.
      1. Set <var>ResultStatus</var> to *inactive*.
   1. Else if <var>ResultRaycastStatus</var> is *raycast requirement unaffected*:
      1. If this input sequence's raycast success flag is not set:
         1. Set <var>ResultStatus</var> to *inactive*.
7. If this input sequence's frame status is not equal to <var>ResultStatus</var>:
   1. Set this input sequence's frame status change flag.
   1. Set this input sequence's frame status to <var>ResultStatus</var>.
8. If <var>ResultStatus</var> is *inactive*, then:
   1. Set this input sequence's last advancement time to <var>ResultDownTimestamp</var>.[^19]
9. Else if <var>ResultStatus</var> is *down*, then:
   1. Set this input sequence's last advancement time to <var>ResultDownTimestamp</var>.[^19]
   1. If this input sequence's frame status change flag is set:
      1. Set this input sequence's [down timestamp](#down-timestamp) to <var>ResultDownTimestamp</var>.
10. Else if <var>ResultStatus</var> is *released*, then:
   1. Let <var>DownTimestamp</var> be the input sequence's down timestamp.
   1. Let <var>ConsumedByMoreSpecificSequence</var> be false.
   1. Let <var>Specificity</var> be the [specificity](#specificity) of this input sequence.
   1. [Clear all progress](#clear-all-progress) for this input sequence.
   1. For every input <var>Input</var> in <var>TerminalInputs</var>:
      1. Let <var>DeviceButton</var> be the device button state for <var>Input</var>.
      1. Let <var>CurrentClaim</var> be <var>DeviceButton</var>'s current-frame claim.
      1. If <var>CurrentClaim</var> has a specificity greater than or equal to <var>Specificity</var>:
         1. If <var>DownTimestamp</var> is prior to <var>CurrentClaim</var>'s timestamp:
            1. Set <var>ConsumedByMoreSpecificSequence</var> to *true*.
            1. Break.
   1. If <var>ConsumedByMoreSpecificSequence</var> is *false*, then:
      1. Set this input sequence's frame status [back] to released.
      1. Set this input sequence's down timestamp [back] to <var>DownTimestamp</var>.
      1. For every input <var>Input</var> in <var>TerminalInputs</var>:
         1. Let <var>CurrentClaim</var> be InputControl's current-frame claim.
         2. If <var>CurrentClaim</var> has a specificity lower than <var>Specificity</var>:
            1. Set <var>CurrentClaim</var>'s specificity to <var>Specificity</var>.[^20]
            1. Set <var>CurrentClaim</var>'s timestamp to <var>CurrentTimestamp</var>.

[^17]: By making this exception, we allow the user to bind actions directly to range constraints, e.g. binding "Turn Camera" to an Xbox controller's right stick without the need for any buttons to be pressed. Of course, this only actually works if the button press type used for the bind is Hold.
[^18]: This case ensures that if a non-zero number of buttons are currently down, but all of them went down prior to the input sequence's last advancement time, then interruption checks will skip all of them.
[^19]: This is done unconditionally; we want to overwrite the last advancement time even if <var>ResultDownTimestamp</var> precedes it. This is because if the user releases some keys in a concurrent input sequence group, they effectively "rewind" back through the group.
[^20]: We don't need to avoid claiming the inputs that were released on the current frame, as our pending claim will be discarded rather than applied if the inputs are still not down on the next frame.

### Clear all progress
In order to clear all progress for an input sequence:

1. Set the input sequence's [frame status](#frame-status) to inactive.
2. Set the input sequence's [down timestamp](#down-timestamp) to zero.
3. Set the input sequence's [last advancement time](#last-advancement-time) to zero.
1. Clear the input sequence's [raycast success flag](#raycast-success-flag).
4. Clear all progress for the input sequence's root group.

In order to clear all progress for an input sequence group:
1. Set the group's <var>CurrentItemIndex</var> to 0.
2. Recursively clear all progress for group's children.

## Input sequence group update algorithm
This algorithm runs on a single input sequence group, and works by potentially changing the group's frame status. The algorithm itself returns:

* A [frame status](#frame-status).
* A down timestamp.
* The number of currently-down input controls seen by the algorithm; you could regard this as a "progress" value, with the containing input sequence's specificity as the overall maximum.
* A "raycast status:" an enumeration indicating whether the [raycast requirement](#raycast-constraint) was satisfied on this frame; allowed values are *raycast requirement met*, *raycast requirement unaffected*, and *raycast requirement failed*.

The returned timestamp is the most recent timestamp at which the user progressed through entering the input sequence by pressing any input control down.

The algorithm receives parameters <var>CurrentTime</var>, <var>LastAdvancementTime</var>, Device, boolean <var>RaycastAlreadyPassed</var>, and optional parameter <var>PreviousSiblingTime</var>, and works as follows:

1. If <var>PreviousSiblingTime</var> is unspecified, it defaults to a zero timestamp.
2. Let <var>Group</var> be the input sequence group we are running the algorithm on.
3. Clear <var>Group</var>'s frame status change flag.
4. If <var>Group</var> is a single input control, then:
   1. If that input control is released, then return: the *released* frame status; the input control's down timestamp; and a count of 0.
   1. If that input control is down, then:
      1. If <var>Group</var> is its containing input sequence's [raycast-associated button](#raycast-constraint):
         1.	If the input control was pressed down on this frame:
            1. Let <var>RaycastResult</var> be the result of a raycast performed on this frame. [The result should be cached by the input device to which this input control belongs, and associated with said control.](#raycast-result)
            1. If the input sequence's [raycast requirement](#raycast-constraint) is not satisfied by <var>RaycastResult</var>, then:
               1. Return: the *inactive* frame status; a zero timestamp; a count of 0; and *raycast requirement failed*.
            1. Else:
               1. Return: the *down* frame status; the input control's down timestamp; a count of 1; and *raycast requirement met*.
         1. Else if the containing input sequence's [raycast success flag](#raycast-success-flag) is set:
            1. If the raycast requirements are per frame, or if they fail if the target changes:
               1. Let <var>FrameRaycastResult</var> be the result of a raycast performed on this frame.
               1. If the raycast requirement is per-frame:
                  1. If the raycast requirement is not satisfied by <var>FrameRaycastResult</var>, then return: the *inactive* frame status; a zero timestamp; a count of 0; and *raycast requirement failed*.
               1. If the raycast requirement fails if the target changes:
                  1. Let <var>ButtonRaycastResult</var> be [the raycast result associated with this input control](#raycast-result).
                  1. If <var>FrameRaycastResult</var> and <var>ButtonRaycastResult</var> hit different targets, then return: the *inactive* frame status; a zero timestamp; a count of 0; and *raycast requirement failed*.
      1. Return: the *down* frame status; the input control's down timestamp; a count of 1; and *raycast requirement unaffected*
   1. Assert: the input control is inactive.
   1. Return: the *inactive* frame status; a zero timestamp; and a count of 0.
5. If <var>Group</var>'s type is *concurrent and ordered*, then:
   1. Let <var>RaycastStatus</var> be *raycast requirement unaffected*.
   1. Let <var>PreviousTimestamp</var> be <var>PreviousSiblingTime</var>.
   1. Let <var>CountDown</var> be 0.
   1. Let <var>AnyInactive</var> be false.
   1. Let <var>AnyReleased</var> be false.
   1. Let <var>Index</var> be 0.
   1. For each item <var>Item</var> in <var>Group</var>, using <var>Index</var> to iterate:
      1. Run this algorithm on <var>Item</var>, passing <var>PreviousTimestamp</var> as the value of the nested <var>PreviousSiblingTime</var> invocation. Let <var>ResultStatus</var> be the returned frame status; let <var>ResultDownTimestamp</var> be the returned timestamp; let <var>ResultCount</var> be the returned count; and let <var>ResultRaycastStatus</var> be the returned raycast status.
      1. If <var>ResultRaycastStatus</var> is not *raycast requirement unaffected*, then set <var>RaycastStatus</var> to <var>ResultRaycastStatus</var>.
      1. If <var>ResultStatus</var> is not released:
         1. If <var>ResultDownTimestamp</var> is less recent than <var>PreviousTimestamp</var>, then:
            1. Set <var>AnyInactive</var> to *true*.
            1. Break.
      1. If <var>ResultStatus</var> is inactive, then:
         1. Set <var>AnyInactive</var> to *true*.
         2. If <var>ResultDownTimestamp</var> is greater than <var>PreviousTimestamp</var>, then set <var>PreviousTimestamp</var> to <var>ResultDownTimestamp</var>.[^21]
         3. Increase <var>CountDown</var> by <var>ResultCount</var>.
         4. Break.
      1. Else if <var>ResultStatus</var> is released:
         1. If <var>ResultDownTimestamp</var> is not a zero timestamp, and is older than <var>PreviousTimestamp</var>:
            1. Set <var>AnyInactive</var> to *true*.
            1. Break.
         2. Set <var>AnyReleased</var> to *true*.
      1. Else if <var>ResultStatus</var> is down:
         1. Set <var>PreviousTimestamp</var> to <var>ResultDownTimestamp</var>.
      1. Increase <var>CountDown</var> by <var>ResultCount</var>.
   1. If <var>AnyInactive</var> is *true*:
      1. Assert: <var>Index</var> is in bounds.
      1. For each item <var>Item</var> in <var>Group</var> starting after[^22] <var>Index</var>:
         1. Clear all progress for <var>Item</var>.
      1. Return: the *inactive* frame status; <var>PreviousTimestamp</var>; <var>CountDown</var>; and <var>RaycastStatus</var>.
   1. If <var>AnyReleased</var> is *true*, then return: the *released* frame status; <var>PreviousTimestamp</var>; <var>CountDown</var>; and <var>RaycastStatus</var>.
   1. Return: the *down* frame status; <var>PreviousTimestamp</var>; <var>CountDown</var>; and <var>RaycastStatus</var>.
6. Else if <var>Group</var>'s type is *concurrent and unordered*, then:
   1. Let <var>CountDown</var> be 0.
   1. Let <var>AnyInactive</var> be false.
   1. Let <var>AnyReleased</var> be true.
   1. Let <var>MostRecentlyDown</var> be a zero timestamp.
   1. Let <var>RaycastStatus</var> be *raycast requirement unaffected*.
   1. For each item <var>Item</var> in <var>Group</var>:
      1. Run this algorithm on <var>Item</var>. Let <var>ResultStatus</var> be the returned frame status; let <var>ResultDownTimestamp</var> be the returned timestamp; let <var>ResultCount</var> be the returned count; and let <var>ResultRaycastStatus</var> be the returned raycast status.
      1. If <var>ResultRaycastStatus</var> is not *raycast requirement unaffected*, then set <var>RaycastStatus</var> to <var>ResultRaycastStatus</var>.
      1. Increase <var>CountDown</var> by <var>ResultCount</var>.
      1. If <var>ResultDownTimestamp</var> is more recent than <var>MostRecentlyDown</var>, then set <var>MostRecentlyDown</var> to <var>ResultDownTimestamp</var>.
      1. If <var>ResultStatus</var> is inactive, then:
         1. Set <var>AnyInactive</var> to *true*.
      1. Else if <var>ResultStatus</var> is released, then:
         1. Set <var>AnyReleased</var> to *true*.
   1. If <var>AnyInactive</var> is *false*, then:
      1. If <var>AnyReleased</var> is *true*, then return: the *released* frame status; <var>MostRecentlyDown</var>; <var>CountDown</var>; and <var>RaycastStatus</var>.
      1. Return: the *down* frame status; <var>MostRecentlyDown</var>; <var>CountDown</var>; and <var>RaycastStatus</var>.
   1. Return: the *inactive* frame status; <var>MostRecentlyDown</var>; <var>CountDown</var>; and <var>RaycastStatus</var>.
7. Else if <var>Group</var>'s type is *separate and ordered*, then:
   1. Let <var>Interrupted</var> be the result of running an interruption check on <var>Group</var>.
   1. If <var>Interrupted</var> is *true*:
      1. [Clear all progress](#clear-all-progress) for <var>Group</var>.
      1. Return: the *inactive* frame status; a zero timestamp; and 0.
   1. Let <var>CurrentItemIndex</var> be a persistent run-time-only state value stored on <var>Group</var>. It should be the index of an item in <var>Group</var>. It should be initialized to 0.
   1. Let <var>CurrentItem</var> be the item in <var>Group</var> at index <var>CurrentItemIndex</var>.
   1. Run this algorithm on <var>CurrentItem</var>. Let <var>ResultStatus</var> be the returned frame status; let <var>ResultDownTimestamp</var> be the returned timestamp; let <var>ResultCount</var> be the returned count; and let <var>ResultRaycastStatus</var> be the returned raycast status.
   1. If <var>CurrentItem</var> is the first child of <var>Group</var>:
      1. If <var>ResultDownTimestamp</var> is older than <var>PreviousSiblingTime</var>, then return: the *inactive* frame status; a zero timestamp; a count of 0; and <var>ResultRaycastStatus</var>.
   1. If <var>ResultStatus</var> is down:
      1. If <var>CurrentItem</var> is the last item in <var>Group</var>:
         1. Return: the *down* frame status; <var>CurrentTimestamp</var>[^23]; <var>ResultCount</var>; and <var>ResultRaycastStatus</var>.
      1. Return: the *inactive* frame status; <var>CurrentTimestamp</var>; <var>ResultCount</var>; and <var>ResultRaycastStatus</var>.
   1. Else if <var>ResultStatus</var> is *released*:
      1. Increment <var>CurrentItemIndex</var> by 1.
      1. If <var>CurrentItem</var> is the last item in <var>Group</var>:
         1. Set <var>CurrentItemIndex</var> to 0.
         2. Return: the *released* frame status; <var>ResultDownTimestamp</var>; a count of 0; and <var>ResultRaycastStatus</var>.
      1. Else:
         1. Return: the *inactive* frame status; <var>CurrentTimestamp</var>; <var>ResultCount</var>; and <var>ResultRaycastStatus</var>.
   1. Else if <var>ResultStatus</var> is *inactive*:
      1. **[Don't let too much time pass between separated keystrokes.]** If <var>CurrentItemIndex</var> is greater than zero:
         1. Let <var>ComparisonTime</var> be whichever timestamp is more recent among <var>LastAdvancementTime</var> and <var>ResultDownTimestamp</var>.[^24]
         2. Let <var>Elapsed</var> be the difference between <var>ComparisonTime</var> and <var>CurrentTime</var>.
         3. If <var>Elapsed</var> is greater than KeySequenceInputExpiryTime, then:
            1. [Clear all progress](#clear-all-progress) for <var>Group</var>.
            1. Return: the *inactive* frame status; a zero timestamp; a count of 0; and <var>ResultRaycastStatus</var>.
      1. Else, return: the *inactive* frame status; <var>ResultDownTimestamp</var>; <var>ResultCount</var>; and <var>ResultRaycastStatus</var>.
   1. Return: the *inactive* frame status; whichever timestamp is more recent among <var>LastAdvancementTime</var> and <var>ResultDownTimestamp</var>; <var>ResultCount</var>; and <var>ResultRaycastStatus</var>.
8. Assert: unreachable.

[^21]: If <var>ResultStatus</var> is inactive, that doesn't necessarily mean that <var>Item</var> is *entirely* inactive; it could be a nested group with only some of its contents currently being *down*. As such, we still want to capture the timestamp it returns, so we can update the [last advancement time](#last-advancement-time) of the containing input sequence.
[^22]: We here want to clear progress on any *separate and ordered* children if the user "rewinds" back before them by releasing keys in this group that would’ve preceded them. Again, however: the first-seen *inactive* child of <var>Group</var> may be "partially down," so we don't want to clear its progress in case it is or contains a *separate and ordered* group. We clear everything *after* the first-seen inactive group.
[^23]: We enforce a limit on how much time may elapse between fully entering the child items of a *separate and ordered* input sequence group. We rely on the containing input sequence's [last advancement time](#last-advancement-time) to check the elapsed time, and that timestamp is updated based on the timestamp returned by the input sequence group update algorithm. We want the limit to apply to the time between entering child items; we don't want to count the time that any child time spends pressed down; so we always update the last advancement time to the current time while any child item is down.
[^24]: This ensures that if <var>CurrentItem</var> is "partially down," we still take the time of its last keypress &mdash; which may have occurred on this frame &mdash; into account.

## Input sequence range constraint check
This check is invoked by the bind list update algorithm, and not by the input sequence update algorithm, so that we can prevent range constraints from interfering with handling of the Press-blocks-Hold conflict resolution rule.

This check takes an input device handler, Device, as input, and returns a Boolean.

1. Let <var>RangeState</var> be an uninitialized tri-state enum with values zeroed, stale, and active.
2. If the input sequence requires activation of a range input control:
   1. If that control is zeroed:
      1. If that control is stale, then set <var>RangeState</var> to stale; else, zeroed.
   1. Else:
      1. Set <var>RangeState</var> to active.
3. Else:
   1. Return true.
4. If <var>RangeState</var> is zeroed, then return false.
5. Return true.

<span style="page-break-after: always"></span>

# Rules and cases we should revisit
In general, rules regarding *separate and ordered* groups are not considered high-priority at this time, as no such groups exist in any of the default control schemes that we wish to ship with. Even where potential solutions are obvious and are documented here, I'd have to think through their possible side-effects some more, set up test cases, and so on &mdash; and each such change would need to be considered in the context of the others &mdash; and right now I don't deem any of that necessary in order to ship a minimum viable product.

## Consider allowing concurrent advancement through separate and ordered groups
Consider the following input sequence:

* Press &lt;A + S + D + F&gt;

A particularly fast typist may press the S key down while they're still releasing the A key, such that A and S are concurrently down. As separate and ordered groups only advance when the current item is released, this would fail the bind; users must take care to release one child before pressing the next.

It may be worth considering an alternate design wherein if multiple consecutive children (starting at the current item) are down, we advance to (but not past) the last of them immediately. This would require some careful state management, however; we'd have to test group items successively starting at the current item and stopping at any inactive child; if a child is inactive and its previous sibling is down, then we may have to reset state on the inactive child on that frame, to avoid causing issues with any nested separate and ordered groups. Moreover, the potential interactions that this change may have with interruption checks are unclear.

## Separate and ordered groups don't remember when the previous item went down
Consider the following input sequence:

* Press (A + &lt;B + A&gt;)

If you press and hold A, press and release B, and then release A, the input sequence will activate, even though the children of the separate and ordered group are being activated out of order. This occurs because the separate and ordered group tracks its current item, but doesn't track when its previous item went down; therefore, when B is released and A becomes the current item, it sees that A is down but can't tell that it went down before B was down.

In general, you can complete a separate and ordered group by pressing the keys down in almost any order, so long as the following constraints are met:

* All keys except the first must go down before the first goes down.
* All keys must be released in order.
* No additional keys may go down once the first key has gone down.

This issue could be remedied by having separate and ordered groups remember the timestamp at which their previous item went down. The current item's down timestamp could then be compared, and a frame status of *inactive* could be returned as necessary.

## Separate and ordered groups don't fail if the first key goes down concurrently with another key
Consider the following input sequence:

* Press &lt;J + K&gt;

If you were to enter &lt;[J + A] + K&gt;, you would not interrupt the sequence, even if you released A after releasing J and before pressing K.

Consider the following input sequence:

* Press &lt;A + S + J&gt;

If you were to enter &lt;[A + F] + S + J&gt;, you would not interrupt the sequence. However, if you were to enter &lt;A + [F + S] + J&gt;, you would fail the sequence.

This happens because the [interruption check](#input-sequence-group-interruption-check) sub-algorithm explicitly returns false (no interruption) when evaluating a separate and ordered group whose <var>CurrentItemIndex</var> is zero, in order to avoid wasted work when evaluating a separate and ordered group that the user hasn't begun inputting yet. Interruption checks run before updating a separate and ordered group's current item, which means that we don't know if the current item is down; and so if the current item is the first item, we can't distinguish between a separate and ordered group that the user hasn't even started inputting yet, and a separate and ordered group whose first child is being inputted or is fully down.

To fix this, we'd have to consider updating the current item before running the interruption check. We'd then run the interruption check only if we're past our first item, or if the update we just ran returned a frame status of *down* (and isn't disqualified by the previous-sibling timestamp or other factors); and we'd have the interruption check *not* abort immediately if we're still on our first item.

<span style="page-break-after: always"></span>

# Redundant, semi-possible, and impossible input sequences
## Redundant input sequences
### Single-control groups
Any group that contains only a single input control can be unwrapped without changing the behavior of the input sequence. For example, [A + &lt;B&gt;] is equivalent to [A + B].

### [A + A + B] = [A + B]
Concurrent and ordered groups require that each child item have gone down with or after its previous sibling. If a child is identical to its previous sibling (and is not a separate and ordered group), then it will always fulfill this requirement, as it will always go down at the same time as itself.

A variant case exists with nested *concurrent and ordered* groups, as in the form [A + [A + B]].

Notably, however, identical children within a *concurrent and ordered* group [may not always fulfill this requirement](#semi-poss-seq-co-with-distant-identical-children). Moreover, while this rule holds for nested concurrent groups, [it does not hold for nested separate and ordered groups](#imp-seq-identical-so-siblings-in-co).

### (A + A + B) = (A + B)
Concurrent and unordered groups require that each child item be down at the same time, and impose no requirements as to ordering. Therefore, if any two children in the group are identical, then one of them is redundant and can be removed without changing the group's behavior.

Cases (A + A + B), (A + B + A), and (B + A + A) are all identical, as are variant cases (A + (A + B)) and so on.

This rule holds for nested groups, including nested separate and ordered groups; that is, (A + <B + C> + <B + C>) has equivalent behavior to (A + <B + C>).

### (A + [A + B]) = [A + B]
The inner group requires that A and B go down on the same frame, and that A have gone down first. The outer group requires that A be down at the same time as the inner group, regardless of ordering. Therefore, this group is identical to [A + B]; the entire outer group is redundant.

### (A + &lt;B + A&gt;) = &lt;B + A&gt;
The inner group requires that B be pressed and released, and then A be pressed; at this point, the inner group is considered "down," and remains so for as long as the A key is down. The outer group requires that A and the inner group be "down" at the same time. Therefore, the entire outer group is redundant.

Note that this rule is sensitive to the ordering of the inner group's contents; [different orders produce different results](#semi-poss-seq-uo-with-double-leading-for-so-child).

### [A + (B + A)] = (A + B)
This is counterintuitive but sensible. The outer group requires that the A key go down before or at the same time as the inner group. The inner group requires that A and B go down in any order, and the group as a whole is considered to have "gone down" when all of its items are down, reporting its down timestamp as the most recent of its children's down timestamp. Thus, if you press A and then B, then the inner group will go down when B goes down, which is after A; whereas if you press B and then A, then the inner group will go down when A goes down, which is at the same time as A.

## Semi-possible input sequences
These input sequences appear to be impossible at first glance, but can be completed via highly specific sequences or timings of inputs.

### [A + B + A]<span id="semi-poss-seq-co-with-distant-identical-children"></span>
Concurrent and ordered groups require that each child item have gone down with or after its previous sibling, so this input sequence is possible if A and B go down on exactly the same frame. Otherwise, however, the input sequence is impossible, as A would be required to have simultaneously gone down both before and after B, without having been released in the interim.

### (A + &lt;A + B&gt;) = [&lt;A + B&gt; + A]<span id="semi-poss-seq-uo-with-double-leading-for-so-child"></span>
This input sequence can be completed if you press and release A, press and hold B, and then press and hold A.
Note that this rule is sensitive to the ordering of the inner group's contents; different orders produce different results.

## Impossible input sequences

### [A + &lt;B + C&gt; + &lt;B + C&gt;]<span id="imp-seq-identical-so-siblings-in-co"></span>

The inputs required to complete this sequence are as follows:

* A must go down before all other involved keys, and must remain down
* B must go down
* B must be released
* C must go down after the previous B-press, and must remain down
* B must go down after C
* B must be released
* C must go down after the previous B-press

The sequence cannot be completed. The first C-press must be held until the end of the sequence, but the last inner group requires a second C-press that comes after the second B-press, which in turn requires releasing C &mdash; interrupting the sequence.

<span style="page-break-after: always"></span>

# Concepts in other input systems

## Windows accelerator keys

In Windows, an <dfn>accelerator key</dfn> is an input mapping consisting of zero or more modifier keys, and one non-modifier key. The available <dfn>modifier keys</dfn> are Alt, Ctrl, and Shift. When an accelerator key has been defined, you can activate it by pressing and holding the mapped modifier keys in any order and then, while keeping those keys held, pressing the mapped non-modifier key; the accelerator key's mapped function will activate when the non-modifier key first goes down.

You cannot activate an accelerator key if any modifier keys other than the mapped modifier keys are pressed down. For example, pressing and holding Ctrl + Shift + N will not activate a Shift + N accelerator key, even if there exist no accelerator keys for Ctrl + N or Ctrl + Shift + N.

You can activate an accelerator key if non-modifier keys other than the mapped non-modifier key are pressed down. For example, pressing and holding Shift + B + N will activate a Shift + N accelerator key. If a Shift + B accelerator key exists, then both accelerator keys activate in the order their mapped non-modifier keys are pressed.

Windows accelerator keys differ from DovahKit's [input sequences](#input-sequence) in a few ways:

* Accelerator keys use key sequences of a fixed structure: the mapping as a whole is ordered and concurrent, and begins with an unordered group of input controls followed by one single input control.
* Per the above, accelerator keys must have exactly one non-modifier key mapped.
* Windows hardcodes specific modifier keys: Alt, Ctrl, and Shift. This is not suitable for DovahKit, which must support a range of input devices, such as gamepads, where no fixed system-level modifier keys exist.

The non-modifier key in an accelerator key is akin to [terminal inputs](#terminal-inputs) in DovahKit's binds.
