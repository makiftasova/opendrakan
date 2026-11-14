# odOsg — OpenSceneGraph renderer, OpenAL audio, and the main executable

Builds the **`odOsg`** executable (only when `BUILD_OSG_RENDERER=ON`). Headers: `include/odOsg/`. Namespace `odOsg`.
Implements the abstract interfaces from odCore; links `odCore` and `dragonRfl` (whole-archive).

| File / dir | Purpose |
|---|---|
| `Main.cpp` | Program entry. Parses CLI flags, finds engine root (`Dragon.rrc`), constructs `DbManager`, `RflManager` (+`DragonRfl`), OSG `Renderer`, OpenAL `SoundSystem`, `InputListener`, Bullet physics, `Server` and `Client`; optionally inserts a `LocalTunnel`; runs server on its own thread, client on main; handles SIGINT. |
| `Settings.cpp` | `odOsg::Settings`: loads `odOsg.cfg` (INI, via `od::ConfigFile`) — `[video]` width/height/fullscreen/screen/fov (horizontal, default 90 like the original game), `[audio]` master_volume. Add new options here + in `resources/odOsg.cfg.example`. |
| `InputListener.cpp` | Translates OSG GUI events (keyboard/mouse) into `odInput::InputManager` calls. |
| `render/Renderer.cpp` | `odRender::Renderer` impl: viewer/window, scene graph roots (world vs GUI render space), creating handles, loading shaders (`ShaderFactory`). |
| `render/Model.cpp`, `ModelBuilder.cpp` | Build OSG geometry from `odDb::Model` data (vertices, UVs, bone weights, texture lookups). |
| `render/Rig.cpp` | Skinned/rigged mesh driven by `odAnim` skeletons. |
| `render/Geometry.cpp`, `Group.cpp`, `Handle.cpp`, `Camera.cpp`, `Image.cpp`, `Texture.cpp` | Thin wrappers over OSG nodes/textures implementing the odRender interfaces. |
| `render/LightState.cpp` | Per-object light uniform state fed from physics-determined light influence. |
| `render/PhysicsDebugDrawer.cpp` | Draws Bullet debug lines (`-p` flag). |
| `render/ShaderFactory.cpp` | Loads/compiles shaders from `resources/shader_src/` (copied next to the binary at build time). |
| `audio/OpenAlContext.cpp`, `SoundSystem.cpp`, `Source.cpp`, `Buffer.cpp`, `StreamingSource.cpp` | OpenAL implementation of `odAudio` interfaces; streaming source used for music. |
| `audio/music/FluidSynth.cpp` | MIDI/DLS synth via FluidSynth for DirectMusic segments; `DummySynth` (silent) and `StupidSineSynth` (debug) are fallbacks. |
| `GlmAdapter.h`, `Constants.h` | GLM ↔ OSG vector/matrix conversion; shared constants. |

Notes:
- OSG object lifetimes are intrusive-ref-counted (`osg::ref_ptr`); odCore holds our wrapper handles via `std::shared_ptr`.
- Coordinate conventions differ between Riot, OSG and Bullet; conversion happens in `GlmAdapter.h` and the model builder.
