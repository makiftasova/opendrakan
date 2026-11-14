<!--
Provenance: clean-room behaviour description produced by a separate examiner session (opencode, beam-free model) and checked by
an independent reviewer session for leaked original code/names (verdict CLEAN, after one rewrite). The first version was flagged
DIRTY, was never read by the implementer and was discarded. Implementation work reads only this file.
-->
# Character collision and movement (clean-room description)

## 1. Shape
- Against terrain: the character is effectively a point. Ground height is queried at the character's horizontal position; no character volume is used for terrain. CERTAIN.
- The character origin sits about 0.25 world units above the feet, so the sampled foot point is the origin shifted down by about 0.25. LIKELY.
- Against objects: the character uses the same model bounding-volume hierarchy as other models (a tree of spheres or oriented boxes), with polygon-accurate collision response. LIKELY.
- No separate capsule, box, or sphere radius for the character was observed. not determined.

## 2. Ground
- Ground height is obtained by bilinear interpolation of four cell-corner heights in a terrain layer grid. CERTAIN.
- Terrain cells are 1 world unit square; internally positions are scaled by 2048 units per world unit, so cell indices use a 1/2048 factor. CERTAIN.
- Each object stores an associated terrain-layer index. Sampling uses that layer first. CERTAIN.
- If the sample point lies outside the associated layer's rectangle or in a hole cell, the engine searches stacked layers from top to bottom for the first layer whose rectangle contains the point and whose sampled height is not above the current vertical position. LIKELY.
- If no such layer is found, the sampled height is 0. CERTAIN.
- Holes are encoded as a per-cell flag that selects one of two diagonal triangulations for interpolation. CERTAIN.
- The associated layer index is tracked per object and is updated or recovered when it becomes invalid (-1). CERTAIN.

## 3. Steps and slopes
- No explicit maximum automatic step height or maximum walkable slope angle was found in the observed update. not determined.
- Slopes are represented continuously by the interpolated height field, so height changes are smooth rather than stepped. CERTAIN.
- Terrain normals are computed from the height gradient and are available to movement and response. CERTAIN.
- Layer types (floor versus ceiling) can flip the normal depending on type and current height, allowing movement on underside surfaces. LIKELY.
- Hole boundaries use diagonal triangulation, causing sharper height changes there. LIKELY.

## 4. Vertical motion
- When on the ground, the character is snapped to the sampled ground height each update (it sticks to the ground, including downhill). LIKELY.
- When airborne, the vertical coordinate is updated by adding a per-frame vertical velocity. CERTAIN.
- Fall-to-death occurs when the vertical velocity is below -6144 internal units. CERTAIN.
- Landing snaps the character to the ground when it is within 110 internal units of it. LIKELY.
- Fall damage is applied when the fall distance is at least 3000 internal units. CERTAIN.
- If the sampled ground differs from the current position by more than 2048 or 4096 internal units, the vertical position is clamped to the ground. CERTAIN.
- Terminal velocity was not observed. not determined.

## 5. Walls
- Horizontal movement comes from the walk or run animation's root motion. CERTAIN (from context).
- In the observed update, horizontal and input handling is performed before ground sampling and vertical correction. LIKELY.
- Collisions with objects use the model bounding-volume hierarchy and polygon-accurate response; this produces sliding along surfaces rather than a simple stop. LIKELY.
- No ray-cast wall tests were observed in the update. LIKELY.

## 6. Recovery
- If the associated layer index is -1, the engine performs a recovery pass: it recomputes position from terrain sampling, resets vertical state, and re-associates the object with a layer. CERTAIN.
- After teleport or cutscene, a character below or overlapping geometry is moved toward or clamped to the ground using the 2048/4096 internal-unit thresholds and the 110-unit landing snap. LIKELY.
- A character with vertical velocity below -6144 internal units is treated as falling to death rather than recovered. CERTAIN.

## 7. Feel
- Bilinear height interpolation makes walking over terrain and small height changes smooth. CERTAIN.
- A snap tolerance of 23.6 internal units is applied after ground sampling. CERTAIN.
- Inverse kinematics foot placement to terrain is used to keep feet on the ground. GUESS.
- Speed scaling on slopes, camera smoothing, and other feel parameters were not determined. not determined.