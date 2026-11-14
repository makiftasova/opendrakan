<!--
Provenance: clean-room behaviour description. A separate examiner session wrote it by inspecting the original program
statically. A second session checked it for leaks (verdict CLEAN). An earlier version of this description was read by the implementer before the leak review existed. The review found layout notation and names in it, so the examiner rewrote it. The implementer works from this rewritten version only.
-->

# Drakan: per-frame attachment of items to character channels
Clean-room description for re-implementation, derived from observable behavior only.
Labels: CERTAIN = verified at instruction level; LIKELY = strongly inferred; GUESS = plausible, unverified.

## 1. Notation
- A transform is a 3x3 rotation matrix together with a translation vector. Its rotation has rows r0, r1, r2. Applying it to a local vector v gives world = (r0·v, r1·v, r2·v) + t.
- Character world transform: position p, rotation R (a 3x3 matrix), and scale as three diagonal components. The columns of R are C0, C1, C2.
- Channel matrix M: the per-frame matrix of the attach joint ("channel"), with rows m0, m1, m2 and translation t = (tx, ty, tz).
- The attached item's world transform: position w, rotation W (3x3), scale.

## 2. Per-frame placement of the item (question 1)
Every frame, for every attachment, the engine rebuilds the item's world transform from the character's world rotation and the channel's current matrix. It is a composition, not a replacement, and it includes one fixed, constant axis remap:

- Item world position:
    w = p + tx*C1 + ty*C2 + tz*C0
  The channel translation is applied through the character's rotation with its components cyclically permuted: the stored "x" scales C1 (character Y column), "y" scales C2 (character Z column), "z" scales C0 (character X column). Equivalently: w = p + R·(tz, tx, ty).  (CERTAIN, verified term by term)

- Item world rotation, built as three columns:
    W col0 = -( m_{0,0} C0 + m_{0,1} C1 + m_{0,2} C2 )   = -R m0
    W col1 = +( m_{1,0} C0 + m_{1,1} C1 + m_{1,2} C2 )   = +R m1
    W col2 = -( m_{2,0} C0 + m_{2,1} C1 + m_{2,2} C2 )   = -R m2
  In matrix form: W = R · Mᵀ · diag(-1, +1, -1). The sign pattern (-, +, -) is a fixed constant remap applied to every attachment every frame. (CERTAIN, including the explicit negations)

- The constructed columns constitute the rotation W; a consumer reading the transform under the opposite matrix convention sees Wᵀ. (LIKELY for the final storage step; the column construction is CERTAIN)

Practical warning: composing the character's world transform and the channel matrix directly, with no remap, produces exactly the reported symptoms (upside down when standing, backwards when walking). Both remaps above — the sign flip on X and Z and the cyclic translation permutation — are required together.

## 3. Translation offset and scale (question 2)
- There is NO additional constant translation offset. The only positional input is the channel translation t, used with the permutation of section 2. (CERTAIN: nothing else is added in the placement math.)
- Item scale: at attach time the item's scale triple is set equal to the character's scale triple; the item's own prior scale is discarded. The per-frame position and rotation math uses no scale at all. Whether the scale is re-copied on later frames if the character resizes is NOT DETERMINED (a single copy at attach was observed). (CERTAIN for the attach-time copy)
- The character rotation R is used exactly as stored, unnormalized; if scale is folded into its columns, the formulas above still hold as written. (LIKELY)

## 4. Axis convention (question 3)
- diag(-1, +1, -1) is a 180-degree rotation about the Y axis: the item's local X and Z axes point opposite to the channel's X and Z axes; the item's local Y agrees with the channel's Y. (CERTAIN as math; the reading follows directly.)
- Consequence for weapons: the item is authored so that its blade/forward direction is its local -X, with local +Y as up and local Z completing the frame; equivalently, the channel's +X is the item's -X. (GUESS for which physical axis the art uses as "forward"; the sign relationship itself is CERTAIN.)
- The translation permutation means the channel translation is stored in a different component order than the channel's rotation rows: its "z" moves along character X, its "x" along character Y, its "y" along character Z. (CERTAIN behavior; the reason for the ordering is a GUESS — most plausibly the offset is kept in the item-local basis rather than the channel basis.)

## 5. Attach time (question 4)
- On the attach event, before registration, the item's position is immediately overwritten with the character's current position (all stored copies of it). (CERTAIN)
- The item is placed at the joint on the very first frame: within one character update, channels/animation are evaluated first and attachment placement runs after, so the item appears at the joint immediately rather than one frame late, and is re-placed every frame thereafter. (CERTAIN for the ordering)
- A status flag is copied from the character onto the item at attach time, most plausibly meaning "carried / suppress independent collision". (LIKELY)

## 6. Detach (question 5)
- Detaching removes the attachment and releases the item reference. Detaching is also what an attach event with target 0 does. (CERTAIN)
- The item is NOT moved, dropped, thrown, returned to a rest pose, or reoriented. It stays exactly where it was last placed — at the joint, in joint orientation — and becomes a free object from the next update on. Whatever moves it afterward is outside the detach logic. (CERTAIN)

## 7. Two stored matrices per attachment (variation)
Each attachment maintains two channel-derived matrices. One is the animated channel matrix and is used by default; the other appears to be a bind-pose-relative variant, selected per channel by a status flag on the channel. Both are produced by the same composition rule against the character's transform at channel-evaluation time, and the per-frame math of section 2 is identical for either choice; only the source matrix differs. (CERTAIN that two matrices exist and a flag selects between them; LIKELY that the second is the bind-pose-relative one.)

## 8. Update ordering (CERTAIN)
Per character update: channel/animation evaluation first, then attachment placement. Characters are processed in reverse list order. Attachments recurse: an attached item that itself carries attachments has those sub-attachments placed relative to its own, already-updated transform in the same pass.