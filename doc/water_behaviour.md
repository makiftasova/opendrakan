<!--
Provenance: clean-room behaviour description. A separate examiner session wrote it by inspecting the original program
statically. A second session checked it for leaks (verdict CLEAN). The implementer works from this text only.
-->

# Water behavior in the 1999 engine

## 1. How water is identified

- Water is both a terrain surface property and a material/object property. [CERTAIN]
- Type value 49 marks still-water surfaces in shipped levels. [CERTAIN]
- Additional surface types exist for flowing water in eight directions at slow, medium and fast speeds, plus ceiling water with no, up or down current. Their exact type numbers are not determined. [LIKELY]
- Materials can reference water properties. [CERTAIN]
- A flat water layer is not a solid wall: the player can enter it and swim. [LIKELY]
- Water surfaces can be landed on. [CERTAIN]

## 2. Entering and leaving water

- On entering water, the character switches to swimming animations (swim, backswim, tread water, idle underwater). [CERTAIN]
- The exact depth thresholds for wading, swimming and diving are not determined. [not determined]
- How depth is measured relative to the water surface is not determined. [not determined]
- On leaving water for ground, a transition animation from swimming to running plays. [CERTAIN]
- Falling into water plays a dedicated fall-into-water sound. [CERTAIN]

## 3. Swimming movement

- Surface swimming forward speed is 2.0 length units/s. [CERTAIN]
- Surface swimming backward speed is 1.0 length units/s. [CERTAIN]
- Underwater swimming speeds are not determined. [not determined]
- How mouse and keys steer while swimming is not determined. [not determined]
- Buoyancy and gravity in water are not determined. [not determined]
- Whether the character can jump directly out of water is not determined. [not determined]

## 4. Breath

- Air is consumed underwater at 2.5% per second. [CERTAIN]
- At that rate, a full air supply lasts 40 seconds. [CERTAIN]
- Air replenishes at 50% per second when the character can breathe. [CERTAIN]
- A full refill therefore takes 2 seconds. [CERTAIN]
- When air runs out, the drowning animation and drowning sound play. [CERTAIN]
- Drowning results in death. [LIKELY]

## 5. Animations in water

- Swim (forward swimming). [CERTAIN]
- Backswim (backward swimming). [CERTAIN]
- Swim To Backswim / Backswim To Swim (direction changes). [CERTAIN]
- Tread Water. [CERTAIN]
- Idle Underwater. [CERTAIN]
- Idle Underwater to Tread Water. [CERTAIN]
- In-Air To Swan Dive and Swan Dive (diving entry). [CERTAIN]
- Swim To Run (exiting water). [CERTAIN]
- Drown (death in water). [CERTAIN]

## 6. Water surface rendering

- Water surfaces use two textures that scroll in opposite directions. [CERTAIN]
- First texture speed is 0.005 length units/s. [CERTAIN]
- Second texture speed is -0.005 length units/s. [CERTAIN]
- Flow direction is 45 degrees. [CERTAIN]
- Detail texture scaling is 0.25. [CERTAIN]
- Underwater view uses a color filter with red 87, green 136, blue 236 at 25% opacity. [CERTAIN]
- A looped underwater sound plays while submerged. [CERTAIN]
- Fog distances range from 12 to 30 length units. [CERTAIN]
- Whether the surface is fully transparent and the exact blend mode are not determined. [not determined]

## 7. Streams, rivers and waterfalls

- Levels define three current speeds: slow 0.2, medium 0.5 and fast 1.0 length units/s. [CERTAIN]
- Flowing water surfaces have a current direction (north, northeast, etc.). [LIKELY]
- Ceiling water has no current, an up current or a down current. [LIKELY]
- Currents push the player at the above speeds. [LIKELY]
- Waterfalls and streams act as sound emitters (stream and waterfall sounds). [CERTAIN]

## 8. Sounds

- Fall-into-water sound on entering water. [CERTAIN]
- Drowning sound when air is depleted. [CERTAIN]
- Looped underwater sound while submerged. [CERTAIN]
- Water ripple and splash effects accompany water interactions. [CERTAIN]
- Stream and waterfall sounds for flowing water. [CERTAIN]

## 9. Enemies and items in water

- Water deals 0 damage per second. [CERTAIN]
- Water surfaces can be landed on. [CERTAIN]
- A property controls splash/ripple effects on objects. [CERTAIN]
- Whether enemies or items sink or float, and any other special behavior, is not determined. [not determined]
