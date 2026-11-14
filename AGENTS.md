# AGENTS.md — OpenDrakan

OpenDrakan is a C++17 reverse-engineered re-implementation of Surreal Software's **Riot Engine**
(the engine behind *Drakan: Order of the Flame*). It is early-stage and cannot play the game yet.
It needs the original game's data files (`Dragon.rrc`, `*.lvl`, `*.db`, ...) to run; none are in the repo.
License: GPL-3.0. Upstream author is mostly solo; the README says a Rust rewrite was started.

Nested `AGENTS.md` files exist in `src/odCore/`, `src/dragonRfl/`, `src/odOsg/` and `doc/`. Read the one for the area you touch.

## Build / run

```bash
mkdir build && cd build && cmake .. && make -j"$(nproc)"
```

- Requires CMake >= 4.0 and C++17. Deps: zlib, GLM, Bullet (core); OpenSceneGraph, OpenAL (+ FluidSynth for music) for the `odOsg` renderer.
- CMake options: `BUILD_OSG_RENDERER` (default ON), `BUILD_CLASSSTAT` (ON), `BUILD_SRSCED` (ON).
- All binaries and static libs land in the top-level build dir. Shaders from `resources/shader_src/` are copied there.
- `Version.h` is generated into `<build>/include/odCore/Version.h` from git (`GenerateVersion.cmake`, `Version.h.in`).
- CI (`.travis.yml`) just runs cmake + make with g++ and clang++. **There are no automated tests.**
- Run: `odOsg [-v] [-c] [-p] [-t] [-d rate] [-l min:max] [level.lvl]` from the game root (the dir with `Dragon.rrc`; the
  engine also ascends from the level path to find it). No level argument loads the intro level.
  `-t/-d/-l` route client<->server through a simulated in-process network tunnel.
- `.gitignore`d `build/` may exist locally; do not commit it.

## High-level architecture

```
 odOsg (exe)  ── Main.cpp wires everything together
   ├─ odCore (static lib)   engine logic: renderer-/audio-agnostic, defines the abstract interfaces
   ├─ dragonRfl (static lib, whole-archive linked)   game-specific classes/GUI, a "Riot Function Library" (RFL)
   └─ odOsg/*               concrete OpenSceneGraph renderer + OpenAL/FluidSynth audio + input
```

- **Client/Server split**: even singleplayer runs an `od::Server` (authoritative simulation) and an `od::Client`
  (rendering, input, prediction) in one process, connected by `odNet` uplink/downlink connectors (`LocalTunnel` for
  simulated lossy connection, `Queued*Connector` for in-process). `od::Engine` is a variant wrapper for "either one".
- **State replication**: `odState::StateManager` records per-tick snapshots (`Timeline`, `Tick`); `LevelObject` state changes
  and `Event`s go through it. Server sends snapshots/events; client acks. Design notes: `doc/net_protocol.md`.
- **Database/assets**: Drakan stores data in *SRSC* container files (`.db`, `.lvl`, `.rrc`). `odDb::DbManager` loads
  `Database`s; each holds typed assets (`Model`, `Texture`, `Animation`, `Sound`, `Sequence`, `Class`, `Segment`...) created via
  `*Factory`s. Cross-DB references are `AssetRef`s resolved via a `DependencyTable`.
- **RFL plugin system**: level object *classes* (game logic) live in RFLs. `odRfl::RflManager` loads them (statically
  or dynamically); an RFL registers classes whose configurable fields are exposed via the *Probe* visitor pattern
  (`FieldProbe` subclasses: load, prefetch, build...). `dragonRfl::DragonRfl` is the only one, implementing Drakan's `dragon.rfl`.
- **Abstract backends**: `odRender::Renderer`, `odPhysics::PhysicsSystem` (Bullet impl in `odCore/physics/bullet`),
  `odAudio::SoundSystem`, `odInput::InputManager` are interfaces in odCore; odOsg supplies the renderer/audio.
- Ownership relations are diagrammed in `doc/od_ownership.gv` (solid=strong, dashed=weak, dotted=non-owning).

## Namespaces

| Namespace | Where (include/odCore/…) | Purpose |
|---|---|---|
| `od` | top level | Level, LevelObject, Layer, Client, Server, Engine, utilities, file formats |
| `odDb` | `db/` | databases and assets |
| `odRfl` | `rfl/` | RFL interface, class/field introspection |
| `odState` | `state/` | snapshots, events, timeline |
| `odNet` | `net/` | connectors, packets, sockets, protocol |
| `odPhysics` | `physics/` (`odBulletPhysics` for bullet/) | collision, character controller, handles |
| `odRender` | `render/` | renderer interface |
| `odAnim`, `odAudio`, `odGui`, `odInput` | `anim/`, `audio/`, `gui/`, `input/` | self-explanatory |
| `odOsg` | `include/odOsg`, `src/odOsg` | OSG/OpenAL implementations |
| `dragonRfl` | `include/dragonRfl`, `src/dragonRfl` | Drakan game logic |

## Repository map

| Path | What it is |
|---|---|
| `CMakeLists.txt`, `src/*/CMakeLists.txt` | Build. Sources are listed **explicitly** per target — add new `.cpp` files there. |
| `include/` | All headers (public, mirrors `src/` layout). `include/odCore`, `include/odOsg`, `include/dragonRfl`. |
| `src/odCore/` | Core engine library. See `src/odCore/AGENTS.md`. |
| `src/dragonRfl/` | Drakan game classes + GUI. See `src/dragonRfl/AGENTS.md`. |
| `src/odOsg/` | OSG renderer, OpenAL audio, input, **`Main.cpp` (the `odOsg` executable entry point)**. See `src/odOsg/AGENTS.md`. |
| `src/srscEd/Main.cpp` | CLI tool to list/extract records of SRSC files (reverse-engineering aid). |
| `src/classStat/Main.cpp` | CLI tool dumping class definitions (`.odb`/RFL class data and fields). |
| `resources/shader_src/` | GLSL shaders (model, layer, default). |
| `resources/locale_cfg/` | `en/de/it.cfg` locale string tables. |
| `doc/` | Reverse-engineering notes on file formats/protocols. See `doc/AGENTS.md`. |
| `TODO.md` | Feature roadmap (multiplayer, GUI, savegames, skeletal animation/IK, ...). |
| `CONTRIBUTING.md` | PR rules (one feature per PR, rebase over merge commits, link issues). |

## "Where do I change X?" quick guide

| Task | Look at |
|---|---|
| Parse a new SRSC record/asset type | `odCore/SrscFile.*`, `SrscRecordTypes.h`, `db/<Type>.*` + `db/<Type>Factory.*`, `db/Database.cpp` |
| Level loading / object spawning | `odCore/Level.cpp`, `ObjectRecord.*`, `LevelObject.*`, `Layer.*` |
| Add a game class (Drakan RFL class) | `dragonRfl/classes/`, register in `src/dragonRfl/ClassRegistry.cpp` + `CMakeLists.txt`; reference `doc/dragon_rfl_classtypes.txt` |
| Class fields / introspection | `odCore/rfl/Field*.h`, `*Probe.*`, `Class.h` |
| Server tick / game-state authority | `odCore/Server.cpp`, `state/StateManager.*` |
| Client frame loop, interpolation, input | `odCore/Client.cpp`, `input/`, `state/EventQueue.*` |
| Networking / packets | `odCore/net/`, `doc/net_protocol.md` |
| Collision / character movement | `odCore/physics/` (`CharacterController`, `bullet/`) |
| Rendering of models/layers/GUI | interfaces `odCore/render/`; impl `odOsg/render/`; shaders `resources/shader_src/` |
| Skeletal animation | `odCore/anim/`, `db/Animation.*`, `db/SkeletonDefinition.*`, `odOsg/render/Rig.*` |
| Sound / music | `odCore/audio/`, `db/Sound.*`, `db/Segment.*`, `odOsg/audio/`, `doc/music.md` |
| GUI framework / menus | `odCore/gui/` (widgets), `dragonRfl/gui/` (Drakan-specific menus) |
| Startup wiring, CLI flags | `src/odOsg/Main.cpp` |

## Conventions and gotchas

- Style: 4-space indent, Allman braces, `m` prefix for members (`mServer`), `_` prefix for private helpers
  (`_assertValid`), `OD_PANIC()` (`odCore/Panic.h`) for fatal errors, `Logger::info()/verbose()/...` for logging.
  Doxygen-style `@brief` comments on public API. Wiki "Policies and conventions" is the official style reference.
- File headers carry author/date banners (`@author zal`); newer headers use `#ifndef INCLUDE_...` guards.
- Units: world units (wu) vs layer units (lu) — see `odCore/Units.h`.
- Classes in `dragonRfl` that are commented out in `ClassRegistry.cpp` / `CMakeLists.txt` are **not yet ported** to the
  current client/server + state architecture; their sources may not compile. Don't assume they work.
- The RFL library is linked with `--whole-archive` so static registration isn't discarded by the linker.
- Several threads may exist (server, client, input); methods documented as "synchronized" are safe cross-thread, others are not.
  See `odCore/ThreadUtils.h`.
- Never commit game assets or anything copyrighted by Surreal/Warner Bros.
