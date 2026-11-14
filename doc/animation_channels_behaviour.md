<!--
Provenance: clean-room behaviour description. A separate examiner session wrote it by inspecting the original program
statically. A second session checked it for leaks (verdict CLEAN). An earlier version of this description was read by the implementer before the leak review existed. The review found layout notation, code-like fragments and names in it, so the examiner rewrote it twice. The implementer works from this rewritten version only.
-->

# Animation channels in the 1999 Drakan engine — clean-room behaviour description

Sources: static analysis of the original executable and game data. No runtime observation. Labels: CERTAIN / LIKELY / GUESS / not determined.

## 0. Runtime data model

- The model has a tree of joints and a separate list of channels; channels are a strict subset of joints (a humanoid: dozens of joints, only a handful of channels — limb roots, attach points such as a weapon attach point). CERTAIN.
- Each channel controls one joint (or none, denoted -1) and is responsible for a run of vertices in the current morph frame. Channels form a tree; a channel may have child channels and sibling channels; a value of 0 means none. CERTAIN.
- Playback state is kept per joint, not per channel. It includes the current animation id, playback rate, local time, transition-blend information, an "end crossed" flag, and an attachment slot. When a model is attached, every joint's state is initialised to the model's default animation id, rate 1, local time -1. CERTAIN.
- An animation has a duration (which is also its wrap modulus), a set of keyframe groups (one per channel index, each giving a contiguous range of keyframes), a sequence of keyframes where each keyframe holds a time, a 3x3 rotation, and a translation, a channel count identifying the character it belongs to, an optional sound id, and a "hold at end" flag. An animation belongs to a character when its channel count equals the model's channel count. CERTAIN.

## 1. Which joints does "start animation A on channel C" drive?

- The request directly changes only the playback state of C's own joint, and never touches non-channel descendants in the joint tree. The "joint plus all descendants" approach is wrong. CERTAIN.
- Unless the request's "no propagate" flag (value 8) is set, the same request is additionally applied to every channel below C in the channel tree (section 2); each affected channel evaluates A using its own keyframe group (group index = channel index). CERTAIN.
- Channels outside C's subtree are untouched: they keep the animation last started on them (initially the model's default animation). CERTAIN.
- Per-frame pose evaluation walks the channel tree from channel 0, carrying the character's world matrix. Each channel's matrix = (parent channel matrix) times (keyframe matrix of the animation active on its own joint, at that joint's local time). Keyframe selection: the last keyframe with time <= local time, used directly — no keyframe interpolation anywhere except the root channel's translation (section 4). Animations are evidently authored with one keyframe per animation frame, so this steps at the animation's sample rate. CERTAIN / LIKELY (density).
- If the channel's joint has no valid animation (id < 1), the channel's matrix = the parent's matrix unchanged (rigid follow). A channel with joint -1 is evaluated at its parent channel's joint. CERTAIN.
- Vertices are not posed by joints directly: for the model's current morph frame, each channel owns a run of vertices (or a weighted list, splittable between channels); final vertex position = channel matrix applied to the vertex position in the current morph-frame vertex list. The animation system also steps the model's morph-frame index when the game requests it (flag-triggered, one frame per step). CERTAIN / LIKELY.

## 2. "Linked channel" and the propagate flag

- The linkage is defined by the model data as a tree over channels. "Linked" therefore refers to this channel tree, not to a single channel, to index order, or to the parent. CERTAIN.
- The propagate flag is part of the request (value 8 set = do not propagate; clear = propagate). CERTAIN.
- Propagation applies the request to all channels below C in the channel tree: it takes C's child channels in order, applying to each child and its entire subtree before moving to the next child. It proceeds downward only. CERTAIN.
- Propagated applications skip root-only handling (section 3) and skip replaying the animation's sound; otherwise they are identical (same rate, start offset, transition). A channel with no joint does not propagate when it is the direct target, but when reached through propagation it still forwards to its child channels. CERTAIN.

## 3. Requests on the root/body channel (channel 0), and conflicts

- Channel 0 is the top of the channel tree (pose walk starts there); it is the "whole body" channel. A request on channel 0 affects other channels only through propagation: without the flag only the root joint changes; all other channels keep playing their own animations. CERTAIN.
- Root-only extras when the requested animation differs from the one currently on the root joint: the character's root-motion bookkeeping is reset (progress cleared, "end crossed" and root-motion markers set); and if a positive start offset is requested while root motion is enabled on any axis, root motion is suppressed for that animation. CERTAIN; the reading that it is suppression is LIKELY. With reversed rate (negative speed) the root's keyframe cache is seeded to the last keyframe. CERTAIN.
- Conflicts: no arbitration at evaluation. Each channel simply plays the animation written into its joint's state most recently; a later request (later event, or later within the same frame) overwrites the earlier one for every channel it reaches. A body animation started with propagation also re-targets weapon/arm channels; the game's character classes then re-issue the sub-channel animations afterwards (character data lists) so poses such as weapon-holding survive. Engine rule CERTAIN; data re-issue practice LIKELY.

## 4. Root motion detail

- Only the root channel's translation is interpolated: a cubic Hermite spline between its keyframes (tangents from neighbouring keyframes); the same value is exported as the character's per-frame world displacement, gated by per-axis enable flags plus wrap/suppress flags. All rotations and all non-root channels use the nearest keyframe unchanged. CERTAIN.

## 5. Invalid channels; "whole skeleton"

- The start request performs no bounds check on the channel index. Cutscene events resolve the channel by scanning the model's channel list for a channel whose joint equals the requested joint; if none, the result is -1. An index that does not refer to a real channel is, in practice, silently ignored. CERTAIN (no bounds check); LIKELY (ignored in practice).
- A channel with no joint makes the whole request body a no-op (no state change, no propagation). There is no special "all joints" channel value: "whole skeleton" = channel 0 with propagation (reaches every channel in the tree). Detaching from a channel with nothing attached is silently ignored. CERTAIN.

## 6. Cutscene (timeline) animation actions

- The timeline event stores a joint index; the engine resolves it to a channel index by scanning the actor's channel list (as in section 5) and issues the same start request. Same list, same semantics. CERTAIN.
- Generated request flags: animation id, rate, start offset and transition flags (value 39), plus a no-sound-replay flag (value 64) when the event carries flag value 256. CERTAIN.
- Start offset = the timeline's current time: the animation's local time is initialised to the timeline clock, so keyframe T of the animation plays when the timeline reaches T — body animations are time-synchronised to the timeline, not played from 0. CERTAIN.
- Transition time = the event's value; on a track's first pass it is 0 (hard snap, no blend). CERTAIN (first-pass condition: LIKELY). Propagation is on by default (value 8 clear). CERTAIN.
- Three 2-bit event fields configure root-motion axis handling (leave off / enable axis / high-bit mode); high-bit meaning LIKELY "frozen/free". CERTAIN / LIKELY.

## 7. Cutscene object-transform keyframes (position + rotation of the actor)

- Each transform keyframe has an orientation as a quaternion, a position vector, and a flag word. Flag values: 256 means ignore position; 512 means ignore rotation; values 16 and 32 (in the low byte) select movement style. CERTAIN.
- Rotation: quaternion SLERP between the orientation stored in the actor's timeline slot and the current keyframe's quaternion, with s = (timeline time - keyframe time) * (1 / duration to the next transform keyframe). Shortest path is enforced: if the dot product of the two quaternions is negative, one quaternion is negated. Exact-opposite quaternions use the special case sin((0.5 - t) * pi) / sin(t * pi); nearly-parallel quaternions fall back to component-wise linear interpolation. So one segment can never turn more than 180 degrees — a full somersault between two keyframes is not representable and must be authored as several keyframes. CERTAIN.
- Position: p = p0 + (p1 - p0) * s, linear. When the current keyframe's style flag value 32 is set (and a next transform keyframe exists), the path is a Catmull-Rom spline through the previous two, current, and next keyframe positions. When the style flags equal 48 (both movement-style flags set), s is eased: the interpolation parameter becomes 0.5 - 0.5 * cos(pi * s) (spline path only; with the 32 flag clear no easing occurs). CERTAIN.
- The interpolated transform (position plus quaternion-to-matrix rotation, object scale divided out) is written to the cutscene object and copied to the attached actor (position and rotation matrix) every frame the track runs. CERTAIN.
- Phase: when a transform keyframe fires at time t_k, the slot records that keyframe's quaternion and position, the time t_k, the rate 1 divided by (t_next minus t_k) (if that duration is zero or negative, for example at a loop wrap, it is taken modulo the timeline loop length and the slot time resets to 0), and the index of the next transform keyframe (found by scanning forward, skipping non-transform events, wrapping around on loop). Each frame the slot renders the rotation as a slerp from the stored orientation to the current keyframe's orientation with parameter s; the position is rendered either as stored position plus (current keyframe position minus stored position) times s (hold style), or as a Catmull-Rom spline from the previous keyframe's position to the current keyframe's position over s in [0, 1] (spline style). CERTAIN. Consequence: with the spline style the rendered position reaches keyframe k's position only when keyframe k+1 fires (a one-interval delay), while the rendered rotation snaps to keyframe k's orientation at t_k itself. Treat this as literal engine behaviour; whether the original data was authored to compensate is not determined.
- Hold/step: when the next transform keyframe has no movement flags, the fired keyframe's transform is applied immediately and statically (final placement). When the current keyframe lacks the 32 movement-style flag, the object holds its last keyframe transform until the next keyframe fires. Before the first transform keyframe fires the mover does not touch the object. CERTAIN / LIKELY (first-keyframe guard).
- s is not clamped in the mover; overshoot is bounded only by when the next event fires. While a cutscene owns an object, engine object flags are adjusted (a cutscene-controlled flag, plus a no-collision flag per track flag); the actor's normal movement is overridden by the copied transform. LIKELY.

## 8. Playback, looping, transition blending (shared by gameplay and cutscenes)

- Every frame each joint's state advances: time increases by the frame delta time times rate, for every joint of every character. CERTAIN.
- Time leaving range ends the pass: if the animation's "hold at end" flag is set, time is clamped to the last frame (minus epsilon) and the rate is zeroed (freeze); otherwise time wraps modulo the animation duration and the "end crossed" bit is set (loop with a finished marker the game can observe). CERTAIN / LIKELY (modulo helper identity inferred). Changing the rate's sign immediately re-evaluates the pass (time re-sync). CERTAIN.
- A start request may specify rate, start offset (initial local time) and transition time; if a transition is requested but not given, the default transition is 0.2 seconds. CERTAIN.
- Transition blending is done on rendered vertices, not joint poses: during the transition window the per-channel skinning mixes the previously rendered vertex positions with the new pose using w_new = (0.1 + 0.9 * m)^2, where m is the smaller of 1 and elapsed divided by T; w_old = 1 - w_new, per channel (weights live in that channel's joint state). When the window ends (or T <= 0) only the new pose is used. CERTAIN.
- Animations may carry a sound id: played (attached to the actor) when the animation starts, and again when the animation on the root channel ends. CERTAIN.
