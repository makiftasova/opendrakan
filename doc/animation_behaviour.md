<!--
Provenance: clean-room behaviour description. A separate examiner session wrote it by inspecting the original program
statically. A second session checked it for leaks (verdict CLEAN). An earlier version of this file was flagged by the
strict leak review (layout notation, code-like fragments and names), so the examiner rewrote it. This is the rewritten version.
The implementer works from this text only.
-->

# Clean-room specification: skeletal animation behaviour (1999 engine)

## Part 1

## 1. Root translation flags ("Mesh" / "Frame" / "None", per axis)

1.1 The flags encode three per-axis modes, one for X, one for Y and one for Z. Each mode is a value from 0 to 3. — CERTAIN

1.2 For each axis the animated object has two mode states: an "apply" state and a "suppress" state. Whenever a mode configuration is applied, all six states are cleared first, then set per axis from the flags. — CERTAIN

1.3 Mode 0 ("Mesh", default when all flags are zero): neither state is set for that axis. Meaning inferred: the root joint's animated translation on that axis applies only to the mesh pose; the object's world position is not changed by it. — structure CERTAIN, meaning LIKELY

1.4 Mode 1 ("Frame"): the apply state is set for that axis. Meaning inferred: the root joint's animated translation on that axis also moves the object's world position. — structure CERTAIN, meaning LIKELY

1.5 Mode 2 ("None") and mode 3: the suppress state is set. Value 3 behaves exactly like value 2; there is no distinct handling of 3. Meaning inferred: the animated translation on that axis is discarded entirely. — CERTAIN that 3 equals 2; meaning LIKELY

1.6 Root rotation is unaffected by these flags; the per-axis machinery exists only for translation. Root rotation is applied to the pose like any other joint rotation. — flags never touch rotation: CERTAIN; standard application: LIKELY

1.7 The root channel's keyframe interpolation fraction (a number between 0 and 1 recording how far between the two surrounding keyframes the previous frame sampled) is cleared when (a) a mode configuration ends with no axis in Frame mode, or (b) a different animation starts on the root channel. Starting a different animation on the root channel also enables two additional root-motion states: one means "add the starting keyframe's translation to the object position once", and the other is a sub-state cleared after that one-time addition. A third root-motion state is enabled when a request supplies a positive start offset while at least one axis is in Frame mode (interpreted as "begin accumulating from that offset"). Whether a looping animation's per-loop wrap adds to this accumulation: not determined. — clearing behaviour CERTAIN; interpretations as labelled

## 2. Playback

2.1 Starting an animation is a bit-flag request; independent bits let a caller change only part of the channel state: start or replace animation, set speed, set start offset, set transition time, propagate to the linked channel, switch without blending. One request may combine several. — CERTAIN

2.2 Each channel tracks its current animation, playback speed, animation-time origin, transition duration, and blend interval. — CERTAIN

2.3 Speed is a signed multiplier; negative speed plays the animation backwards. When the sign of the speed changes on a live channel, the pose is immediately re-solved. When a request sets a negative speed, the channel's current keyframe index is initialised to the last keyframe of the animation. — CERTAIN

2.4 Start offset: the requested "begin play at" time is stored as the channel's animation-time origin. Starting a different animation without an explicit offset resets the offset to zero. The per-frame sampling formula itself was not examined: not determined. Most consistent with the stored state: sampled time equals start offset plus elapsed time multiplied by speed. — LIKELY, not verified

2.5 End-of-animation behaviour (play once versus loop, wrap versus clamp at the last keyframe): not examined. Related certain facts from the timeline system: each sequence track has a loop flag; when a track's event list is exhausted and the flag is set, the track's event index and local time reset to zero and it restarts, otherwise the track reports finished. Also, the engine explicitly reports at runtime that pausing a running sequence is unsupported. — CERTAIN

2.6 Keyframe interpolation (linear translation, spherical versus normalised-linear rotation, short-path sign handling): not examined. One certain related fact: animation translation values are stored in units of 1/2048 world unit and multiplied by 2048 when loaded, so at playback time translations are ordinary world-unit floats.

## 3. Transitions

3.1 When a request starts a different animation on a channel and supplies no transition time, the default transition duration is 0.9 seconds. — CERTAIN

3.2 With a positive transition time on a different animation, the engine first captures state from the outgoing animation (skipped when the request asks for a no-blend switch, or when the new animation equals the current one), then creates a blend window: blend start equals current time; blend end equals blend start plus transition duration; the channel is marked blending until the window passes. The evaluation of the blend was not examined. The natural form, consistent with the stored window: w equals (current time minus blend start) divided by transition duration, clamped to the range 0 to 1; pose equals outgoing pose times (1 minus w) plus incoming pose times w, per joint. — structure CERTAIN, formula LIKELY (not verified)

3.3 Re-triggering the same animation on a channel performs no capture and creates no transition; only the requested speed/start-offset changes are applied. — CERTAIN

3.4 One animation "channel" drives exactly one named channel slot. There is no automatic subtree propagation at start time. However, one request flag makes the same start additionally apply to the slot's single explicitly linked channel slot, and that link can chain further through the same flag. — mechanism CERTAIN; linked chains used to cover sub-trees in practice: LIKELY

3.5 In sequence timelines: a per-track flag forces the transition time of every animation start on that track to zero, and a per-action flag requests a switch without blending (which also skips the outgoing-pose capture). — CERTAIN

## 4. Attachment (timeline "attach" action)

4.1 On an actor's track, the action names a target actor id and a channel index of the attaching actor's skeleton. Target id 0 detaches whatever is attached to that channel. — CERTAIN

4.2 The channel is resolved by matching the channel id against the attaching actor's animation channels; if the channel is not present, the attach or detach is still attempted with an invalid channel index. If the target actor id does not resolve (or has not spawned yet), a runtime message reports it and the action is skipped. — CERTAIN

4.3 At attach time the item's current world position is overwritten with the attaching actor's current position before the attachment is made; the item's previous position is discarded. (A coordinate conversion routine is applied during the copy; see 5.4.) — CERTAIN

4.4 Per-frame derivation of the item's world transform from the joint's animated transform — whether the item keeps an offset from the joint, whether its own position/rotation is replaced or composed, whether joint scale is applied — was not examined: not determined.

4.5 Detach behaviour (item stays where it is / falls / returns to a stored position): not examined. Only the existence of a per-channel-slot detach operation is CERTAIN.

## 5. Other details that will make animations look wrong if missed

5.1 Animation identity and owner binding: an animation id combines an owning class id and an index. Starting an animation validates that the animation's owner matches the animation system's owner and otherwise rejects it with the runtime message "Animation does not belong to this character!"; an id whose owning class has no database entry yields "Bad animation index.". — CERTAIN

5.2 Animation data is streamed and cached, not assumed memory-resident. Each loaded animation is accounted as 8 per keyframe plus 52 per animation entry against a cache budget; when the budget is exceeded (and a settings flag allows), a cache flush evicts entries; a user-facing cache-size setting exists. An implementer should support lazy loading of animation data. — CERTAIN

5.3 Translation is stored in units of 1/2048 world unit and scaled by 2048 at load. The remaining keyframe contents (time and/or rotation data) were not decoded. Each animation has additional data of unknown purpose, grouped in pairs of values. Each animation also references one other animation id (owner-remapped), used when playback starts; interpreted as a transition-source or base-pose reference — GUESS.

5.4 Every write of an object's world position passes through one shared conversion routine. This indicates an axis/handedness conversion between two coordinate systems (for example engine-internal versus world or render space). The conversion exists; the exact axis mapping was not determined. Getting it wrong will show up as swapped or mirrored character placement and attachments. — conversion CERTAIN, axes not determined

5.5 Late timeline events: if the sequence time has already passed an event's scheduled time when it fires, the started animation receives a "begin play at" of its scripted offset plus the elapsed overshoot — late starts resume partway into the animation instead of restarting at the scripted offset. — CERTAIN

5.6 Channel lookup is by channel id against the object's animation channels (entries also chain one link each, see 3.4). A start addressed to a channel id that is not present is silently ignored (no error reported). — channel list CERTAIN, silent ignore LIKELY

## Unresolved after part 1

- Per-frame sampler formula and end-of-animation handling (2.4, 2.5).
- Interpolation method and rotation sign handling (2.6).
- Blend evaluation formula (3.2).
- Per-frame attached-item transform, offset and detach behaviour (4.4, 4.5).
- Loop-wrap accumulation of root motion (1.7).
- Keyframe contents beyond the translation triplet and the additional value-pairs (5.3).
- Exact axis mapping of the position conversion (5.4).

## Part 2: skeletal animation details (1999 engine)

Continuation of the earlier specification. Method unchanged: static inspection of the engine executable. This part covers the previously unresolved items: per-frame attached item transforms and detach, the per-frame sampler and end-of-animation behaviour, keyframe interpolation and transition blending, and root-translation accumulation. It also records new findings relevant to visual correctness, and corrects two statements from part 1.

## 0. Corrections to part 1

0.1 Part 1 described a "stored root-offset value" cleared when a mode configuration ends with no Frame axis and when a different animation starts on the root channel. The cleared value is in fact the root channel's keyframe interpolation fraction (a number between 0 and 1 recording how far between the two surrounding keyframes the previous frame sampled). Clearing it just restarts that memory; it is not an accumulated world offset. — CERTAIN (supersedes 1.7 of part 1)

0.2 The two extra root-motion states enabled when a different animation starts on the root channel (part 1, "root offset accumulator active", GUESS) are now understood: one marks "add the starting keyframe's translation to the object position once", and the other is a sub-state cleared after that one-time addition. See 4.5. — CERTAIN behaviour, interpretation LIKELY

## 1. Attached item transform, per frame

1.1 Per frame, after the character's pose has been built, the engine walks every attachment of every animated object. For each attachment whose joint was evaluated this frame, the item's world transform is recomputed from scratch. — CERTAIN

1.2 Source matrix: the joint's current matrix comes from the character's per-joint matrix palette, after the per-channel correction matrices (5.4) and any IK rotation (5.5) have been applied to it. While the attachment's channel is in a transition blend, this matrix is first replaced by a blend of the item's matrix from the previous frame and the new joint matrix, combined row by row (component-wise linear interpolation of the twelve matrix numbers): blended row equals previous row times (1 minus w) plus joint row times w, with w the channel's incoming blend weight (see 3.5). Rows whose new content is degenerate (length below a tiny threshold) keep their previous value. When the channel is not blending, the joint matrix is used unchanged. — CERTAIN

1.3 Position: the item's world position equals holder position plus holder rotation times joint translation, where joint translation is the translation part of the (possibly blended) joint matrix in the character's model space, and holder rotation is the holder object's current orientation. — CERTAIN

1.4 Orientation: the item's orientation basis vectors are rebuilt each frame from the joint matrix's rotation rows, transformed by the holder's rotation. One of the joint's rotation basis rows is used with inverted sign, and the evidence indicates a second row is also inverted, i.e. the item receives the joint's rotation composed with a fixed 180-degree turn about the remaining axis (a proper rotation, determinant +1). The item's third basis vector is derived and normalised by the placement helpers. The exact naming of which model axis is flipped could not be pinned down. — a fixed re-orientation relative to the joint frame is applied: LIKELY; which axis: not determined

1.5 Replacement, not composition: the item's stored position and orientation values are overwritten each frame. No offset captured at attach time is added, no separate scale factor is multiplied, and no additional fixed rotation beyond 1.4 is applied. The holder's basis vectors are used as stored; if a holder carries scale inside its basis it would carry over, but no explicit scale multiplication exists in the path. — CERTAIN

1.6 Bookkeeping each frame: the item inherits one state flag from the holder (the same flag the attach action set), is marked as moved, and if the item itself has an animation system, its own attachments are processed recursively (nested attachments work). If the attached item object no longer exists, the engine detaches it automatically on that frame. — CERTAIN

1.7 Detach: the detach operation only removes the attachment (compacting the list), fixes up the remaining indices, and releases the item's registry reference. It performs no repositioning: the item keeps whatever world transform the attachment last wrote into it, and becomes a free-standing object again. Any falling, returning or re-placement afterwards is game-object logic outside the animation system and was not determined. — no repositioning in the engine path: CERTAIN; "stays where it is" as the visible result: LIKELY

## 2. Per-frame sampler and end of animation

2.1 Each channel's playback state includes the current animation id, the current animation time, the speed, the start offset, the transition duration, the blend window (start and end absolute times), and the two blend weights. Time is advanced once per frame for every channel of every animated object, before pose building. — CERTAIN

2.2 Advance rule, per channel and per frame: unwrapped time equals current time plus frame delta seconds times speed; sampled time equals unwrapped time modulo duration (remainder, sign preserved); if sampled time is less than zero, add duration. The start offset is simply the initial value of current time (set once when requested; starting a different animation without an explicit offset resets it to zero). So effectively sampled time equals start offset plus accumulated elapsed times speed, modulo duration. — CERTAIN

2.3 Boundary handling. A boundary is crossed when the unwrapped time reaches or passes the duration (forward speed) or zero (negative speed). On crossing:
   - a "reached the end" state is recorded on the channel, observable by game logic;
   - if the animation's per-animation flag says hold-at-end, the channel freezes: the speed is forced to zero and the time is parked at duration minus 0.000001 (play once, then hold on the last frame; the channel keeps the animation);
   - otherwise the animation loops: the time wraps modulo the duration (2.2) and playback continues. For negative speed reaching the start, an already-at-zero channel wraps around to just past the duration instead of freezing.
   — CERTAIN

2.4 At the moment a boundary is crossed, if the animation references a sound id and this channel is the object's designated sound channel, that sound is played, timed from the object's position; a looping animation therefore re-triggers its sound on every loop. The same sound also plays immediately when a different animation starts on the channel (unless the request asked to switch without replacing). — CERTAIN

2.5 Speed is a signed multiplier in seconds of animation time per second of real time. Changing the sign on a live channel immediately re-solves the pose; a request that sets a negative speed initialises the current keyframe index to the last keyframe, so reverse playback starts from the end. — CERTAIN

2.6 Playback types: only the two behaviours of 2.3 exist (hold-at-end and loop), selected by a per-animation flag in the animation data. No other playback type was found in the sampler. Which data value selects it is not determined by name, only by behaviour. — CERTAIN behaviour; field naming not determined

2.7 Pausing: the whole advance and pose-build pass is skipped for an object when a per-object state flag ("animations frozen") is set, or when a global pause switch is active. Frozen objects hold their last pose. — CERTAIN

## 3. Keyframe interpolation and transition blending

3.1 Each keyframe is a complete joint pose: a time value, an orientation basis (3 by 3), and a translation (3 values). Translations are stored in units of 1/2048 world unit and scaled on load. Each channel of an animation is associated with its own sequence of these keyframes. — content CERTAIN (time plus orientation plus translation); exact storage conventions: not determined in detail

3.2 Mesh pose sampling has no interpolation between keyframes. For each joint, the engine finds the last keyframe whose time is at or before the current animation time (incremental forward/backward search per channel, remembering its position between frames) and uses that keyframe's matrix directly; the composed joint matrix equals parent matrix times keyframe matrix (recursion down the joint tree; joints without an active animation inherit the parent matrix unchanged). The same step sampling is used by the engine's standalone query for a channel's transform at a given time of a given animation. Animations are therefore effectively dense pose snapshots (one per frame of the source animation; a per-animation frame-rate setting exists in the game data). — CERTAIN

3.3 Consequence: everything except root translation (which is separately interpolated, see 4.2) is visually stepped between keyframes. Getting this wrong (adding interpolation) will produce smoother-but-different motion; getting the keyframe density assumption wrong will produce visibly steppy motion. — inference from CERTAIN evidence

3.4 Transition blending weights. When a different animation starts with a positive transition time, a blend window is recorded on the channel in absolute engine time (blend start equals the moment of the switch, blend end equals blend start plus transition duration, default duration 0.9 seconds when not supplied). While inside the window: t equals (now minus blend start) divided by transition duration, clamped to the range 0 to 1; w equals (0.1 plus 0.9 times t) squared (incoming weight); incoming weight equals w, outgoing weight equals 1 minus w. Note the curve: the incoming pose starts at weight 0.01, not 0, and rises quadratically (ease-in) to 1. Past the window the blending state is cleared. — CERTAIN

3.5 Confirmed consumer of the weights: the attached-item matrix blend (1.2), which is a component-wise linear blend of whole matrices — not rotation-aware, no quaternion handling, no shortest-path correction. The corresponding blend of the mesh pose itself was not located before analysis stopped; the per-channel correction matrices and the palette path show no weight usage. Whether the mesh pose is blended during transitions, and by what mechanism, is not determined. — attachment blend CERTAIN; mesh-pose blend: not determined

3.6 Rotation interpolation between keyframes: none exists in the playback path (3.2). The only quaternion machinery in the animation code serves the IK layer (axis-angle to quaternion to matrix); no spherical or normalised-linear quaternion interpolation and no shortest-path sign flipping were found in playback. — CERTAIN (absence within the examined paths)

## 4. Root translation in Frame mode: accumulation into world position

4.1 The root joint's keyframe translation is evaluated specially, once per frame, using the two keyframes bracketing the current time on the root channel. Per axis:
   - Mesh mode: the keyframed translation component stays in the root joint's matrix (the whole mesh visually offsets relative to the object origin); the object's world position is not changed by it.
   - Frame mode: the component is removed from the root joint's matrix (zeroed in the pose) and instead becomes a per-frame world displacement for the object.
   - None mode: the component is zeroed in the pose and produces no displacement.
   All three modes are evaluated per axis; the mode meanings of part 1 (1.3-1.5) are thereby confirmed behaviourally. — CERTAIN

4.2 The per-frame displacement in Frame mode is the interpolated root motion: the engine computes the fraction between the bracketing keyframes, f equals (current time minus lower keyframe time) divided by (upper keyframe time minus lower keyframe time), remembers it, and produces a displacement per axis equal to the keyframe-to-keyframe translation difference scaled by the change in fraction since the previous frame, i.e. the object advances by exactly the root motion the animation played this frame. The previous frame's fraction is reused for that difference. — CERTAIN structure; exact arithmetic in degenerate cases (single keyframe, equal times): not determined in full

4.3 The displacement is applied immediately at the end of the root joint's evaluation, inside pose building: object position increases by object rotation basis times displacement, with each axis' displacement multiplied by the object's corresponding basis axis (the object's current orientation transforms the animation-space motion into world space). The mesh rendered that same frame and the object position are consistent. — CERTAIN

4.4 Across loops: accumulation continues. The segment from the last keyframe back to the first keyframe is handled explicitly (the wrap segment's translation difference is used with the wrap-aware bracket selection and the preserved direction), so a looping Frame-mode animation keeps driving the object forward smoothly through each loop boundary; nothing resets at the wrap. — CERTAIN (mechanism observed); full sign handling in every direction combination: LIKELY

4.5 On starting a different animation on the root channel, on the first evaluation of the new animation the starting keyframe's translation is added once, directly, to the per-axis displacements (an immediate snap by the new animation's initial root offset), after which a sub-state clears and playback proceeds by 4.2. A positive requested start offset with at least one Frame axis enables this from that offset. — CERTAIN behaviour; interpretation as an intentional snap: LIKELY

4.6 Overrides: when every axis is None-suppressed, or a separate "root motion fully disabled" state is set (observed being toggled by a timeline action), the root joint's entire translation is zeroed in the pose and no displacement is produced. The per-axis displacement outputs are cleared at the start of every root evaluation. — CERTAIN

## 5. Additional findings relevant to visual correctness

5.1 Rigid vertex skinning. Each model vertex belongs to exactly one joint. Per frame, each bound vertex is transformed by its joint's current matrix (a plain point-times-matrix). There are no per-vertex bone weights and no multi-bone blending. An implementer using smooth skinning will get different flesh deformation near joints. — CERTAIN

5.2 Per-joint matrix palette. The evaluated joint matrices are collected per object each frame into a shared palette consumed by rendering and gameplay. Objects also retain per-channel data from the last skinning pass (likely for hit detection). The palette entries are joint matrices, reinforcing that keyframes are pose matrices rather than rotation/translation pairs. — CERTAIN; gameplay use of the cache: LIKELY

5.3 Skinning variants: two skinning code paths exist, selected by a global switch (likely a quality/LOD setting); both implement 5.1. — CERTAIN two paths; purpose LIKELY

5.4 Per-channel correction: two constant 4 by 3 matrices from the model data are applied around the joint's matrix when the joint is evaluated (a sandwich: correction, joint transform, correction, plus a correction translation). These re-map a channel's coordinate frame (for example aligning a grip or muzzle frame with the model's conventions). Omitting them will misalign anything computed from channels (attachments, spawns, aim). — CERTAIN existence and application; their data source: LIKELY the model file

5.5 IK layer on top of animation. Solvers attached to the object drive named "solve to" channels: a three-joint chain solver (classic two-bone law-of-cosines: angle equals arccosine of ((a squared plus b squared minus c squared) divided by (2 times a times b)), computed from a target point, with joint lengths from the current joint positions) and a single-joint orientation solver. Results are stored per channel as axis-angle rotations, blended in over a duration with weight w equals (cosine of ((1 minus t) times pi) plus 1) divided by 2, and applied to the joint matrix sandwiched between the channel's correction matrices (5.4). Foot planting, head aiming and weapon aiming use this. Skipping it will leave feet and aim as authored. — CERTAIN structure and formulas; which gameplay features drive them: LIKELY

5.6 Joint query API: game code can ask for any channel's transform at an arbitrary time of any animation; it composes from identity down the joint tree with the same step sampling and per-channel corrections, and can also return the joint's velocity. Used for muzzle and grip positioning and similar. — CERTAIN

5.7 Holder velocity tracking: each frame the animation system computes its holder's per-frame displacement (current position minus previous position) and retains the previous position. This is distinct from root motion; likely consumed by game logic (motion-based effects). — computation CERTAIN; consumers not determined

5.8 The object transform comprises a position and three orientation basis vectors; both root motion (4.3) and attachment placement (1.3, 1.4) transform through these basis vectors. Getting the object basis convention wrong corrupts both root motion direction and attachment placement. — CERTAIN

5.9 Axis/handedness: apart from the basis negations in attachment placement (1.4), no explicit axis-swap or handedness conversion was found in the animation playback, root-motion or attachment paths; vectors are copied as-is. The model's matrix convention was not determined. — no runtime axis conversion in these paths: CERTAIN; model-side conventions: not determined

## Unresolved after this pass

- Mesh-pose application of the transition blend weights (3.5).
- Exact degenerate-case arithmetic of root motion (single keyframe, zero-length segments) (4.2).
- Exact axis naming of the attachment orientation flip (1.4).
- Which data value selects the hold-at-end flag (2.6) and the source of the per-channel correction matrices (5.4).
- Whether cached per-channel data at the object is used for hit detection (5.2).
- Model's matrix convention (5.9).