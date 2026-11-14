
The big OpenDrakan TODO list
============================

This list contains all the features and details of OpenDrakan that still need to be implemented or require more work.
Feel free to pick an item and open a ticket for it should you wish to work on a feature.

- Multiplayer
    - Singleplayer already runs a server and a client in one process. Snapshots are sent every tick, and the client
      adapts its interpolation delay to the observed snapshot spacing and jitter.
    - Client-side prediction for the local player is missing. Movement waits for the server's reply.
    - The server loop uses a variable step. A fixed 1/60 s step with an accumulator would be more deterministic.
    - Biggest hurdle to progress yet. Nearly all game logic should interact with this, so until this stands, progress with game logic is stalled
    - Protocol compatibility has low priority. If the original protocol is the source of bugs, or simply can't be reverse-engineered sufficiently, scratch it
    - Since handling multiplayer probably involves some incremental state tracking, this can probably be leveraged for easily implementing savegames
- Drakan GUI
    - GUI framework needs refinement
        - Support for layout building blocks (boxes, scrollable widgets etc.)
        - Animations
    - Text rendering support
    - Widgets required:
        - ~~Health orb~~
        - ~~Main menu~~
        - Dragon power orb
        - Breath meter
        - Loading bar
        - Settings menu (pretty complex)
    - Main menu crystal buttons still look wrong (crystal motion and lighting)
- Savegames
    - Planned next, after the first gameplay section works.
    - Backwards compatibility is almost completely irrelevant here
    - Might be easier to leverage multiplayer code for this
- Skeletal animation code needs work
    - Attachments (weapons) are still wrong. The rule in `doc/attachment_behaviour.md` is not implemented yet.
      Applying it literally to our matrices put the spears upside down. The conventions of our matrices differ from the description.
      To fix this, compare the same cutscene moment across variants (the cutscene timing differs between runs).
      The animated chain matrix already holds the hand's position, and the current code adds the bind matrix on top. That is probably the offset.
    - Animations should follow the channel tree and its propagation flag (`doc/animation_channels_behaviour.md`).
      Currently they start on the whole joint subtree. Not implemented yet.
    - Transitions between animations should blend with the original curve (the sword jumps now).
    - Known glitches: Rynn spins in the air, one leg is missing, and she floats when crouching.
    - Needs support for inverse kinematics
    - Movements of skeleton need to apply to bounding data
    - ~~Needs support for multiple interpolation styles (mostly "no interpolation" for authentic Drakan animations)~~
- Clean-room documents
    - `doc/animation_behaviour.md` is marked DIRTY by the strict leak review (about 40 lines with layout notation, code-like
      fragments and names). It is already in the pushed history. Have the examiner rewrite it, review it again and replace the file.
      This means rewriting history and force-pushing the fork. Ask before doing that.
    - Both examiner and reviewer must follow `doc/clean_room_process.md`. The examiner must not read the re-implementation.
- STOMP sequences
    - Cutscenes play, including sounds, music, attachments and skipping.
    - Unknown message codes: 0, 15, 25 and 52.
    - The bridge cutscene is missing the Wartok spears.
    - The long spear in the peasant scene after the bridge is missing.
    - Doors and gates aren't rendered. The Wartoks break a gate in the opening cutscene, but nothing is visible.
    - Actor rotations should use the shortest-path slerp.
- Renderer synchronization
    - Access synchronization to the rendering subsystem is inconsistently used or even absent in many cases. This needs a
      new efficient and effective concept.
    - The render loop and the update loop are not synchronized. Thus, it is possible for the renderer to render a partially updated scene,
      resulting in things like the skydome stuttering as a frame occurs between updating the sky and camera
        - A possible elegant solution would be to define "update chains" in the renderer, preventing it from rendering objects with update dependencies
          out of order
- Physics performance
    - The physics system is the biggest hog of game logic loop time right now. There might be some optimizations possible, like
      a broadphase that handles layers separately.
    - Since lights are exclusively spherical in nature, they can be even more optimized with an M-Tree broadphase
- RFL classes
    - Most of the class types still need to be implemented
        - Keep in mind that these will likely require close interaction with the multiplayer code
    - Many classes' original behaviour still needs to be researched (will add list of implemented/documented classes soon)
- Layer PVS
    - We shouldn't spawn objects and layers until they are deemed visible by the current layer
    - ~~A stronger association between objects and the layers they are "on" might be necessary~~
        - ~~Should consider objects that hang off the ceiling, too~~
- DLS loading in music subsystem
    - Music plays. ADPCM samples are transcoded to PCM before FluidSynth loads them.
    - The FluidSynth dependency is big. Might want to try a different synth.
    - Some notes sound wrong or play at once. Needs more work.
- Character controllers
    - Character controlled by player
        - First version works: gravity, ground following, walls and sliding. See `doc/collision_behaviour.md`.
        - Missing: jumping, swimming, ladders, fall damage and tuned step height.
        - Scenery whose class is not implemented has no collision.
    - Dragon controlled by player
    - Character controlled by AI
    - Dragon controlled by AI
- Water
    - Done: terrain with water vertices (type 49) isn't solid. The character swims when the water is at least 0.3 deep.
      It floats at the surface, plays the swim animations and moves with the class's swim speeds.
    - Done: diving (look down while swimming) and air. Air drains underwater and refills at the surface, using the class rates.
    - The dive controls (look to dive), the wading depths and the dive speeds are our own guesses. The original's behavior is not verified.
    - Missing: the breath meter, drowning (the player is pulled to the surface at zero air, because there's no health yet),
      flowing water types and currents, the splash and underwater sounds, and the swim-to-run transition when leaving the water.
    - Missing in rendering: the water surface looks like a flat 80% transparent tint. The original draws two textures that scroll in
      opposite directions. Find out which textures it uses. The terrain uses texture 8/270 for water. Also missing: the underwater tint and fog.
      Stream classes are still hidden.
    - See `doc/water_behaviour.md`.
- AI
- SFX handling
    - If something needs to play SFX, right now that means it has to create it's own sound source. While this obviously works, it might
      be sub-optimal for performance. It'd be better to have a priority queue that distributes the SFX across a few sources.
- Native Wayland window
    - OSG 3.6 only supports X11 (GLX), so the game runs through Xwayland.
    - A custom GLFW or SDL window would fix this.
- Pure OpenGL (or even Vulkan) renderer
    - OSG is a pretty heavy dependency and we only use a small subset of it. A better, pure OpenGL renderer shouldn't be too hard to implement
        
- Input
    - Make the keys rebindable. First support a `[keys]` section in `odOsg.cfg`, then add a settings menu.
    - The original game moves with `E`, `S`, `D`, `F` instead of `W`, `A`, `S`, `D`. Use that as the default layout or as a preset.
- Camera
    - Let the player change the distance between the camera and Rynn with the mouse wheel (or with keys).
      The distance is a constant of 3 units in the tracking camera now.
- Debug overlay
    - Shows the frame rate, the player's position, the layer, the cell, the texture and the vertex types below the player.
      Turn it off with `[debug] overlay = 0` in `odOsg.cfg`. Add a key to toggle it.
- Cheats and developer console
    - Add cheats for testing: god mode (for Rynn and Arokh), heal to full health, give items, spawn items near the player.
    - Needs an in-game developer console and text rendering (see the GUI section).
- Subtitles
    - Dialogue text is stored in the game data. Show it on screen, timed with the dialogue sounds.
    - Subtitles are an important accessibility option. Make them configurable (on/off, size).
